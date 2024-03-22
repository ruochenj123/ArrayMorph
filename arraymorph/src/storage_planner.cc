#include "storage_planner.h"
#include <fstream>
#include <sstream>
#include <algorithm>

int newSqrt(int m, int n) {
	if (n == 1)
		return m;
	if (n == 2)
		return (int)sqrt(m);
	int l = 1;
	int r = (int)sqrt(m);
	while(l <= r) {
		int mid = (l + r) / 2;
		int cur = (int)pow(mid, n);
		if (cur == m)
			return mid;
		else if (cur > m)
			r = mid - 1;
		else
			l = mid + 1;
	}
	return min(l, r);
}
vector<hsize_t> GenerateChunkShape(vector<hsize_t> &dset_shape, int data_size,
	int chunk_size) {
	int ndims = dset_shape.size();

	vector<vector<int>> ordered_dset_shape(ndims);
	for (int i = 0; i < ndims; i++)
		ordered_dset_shape[i] = {dset_shape[i], i};
	sort(ordered_dset_shape.begin(), ordered_dset_shape.end());

	vector<hsize_t> chunk_shape(ndims);
	int nelements = chunk_size * 1024 * 1024 / data_size;
	int cube_size = newSqrt(nelements, ndims);

	int i = 0;
	while (ordered_dset_shape[i][0] < cube_size) {
		int idx = ordered_dset_shape[i][1];
		chunk_shape[idx] = ordered_dset_shape[i][0];
		i++;
		nelements /= chunk_shape[idx];
		cube_size = newSqrt(nelements, ndims - i);
	}
	for (int j = i; j < ndims; j++) {
		int idx = ordered_dset_shape[j][1];
		chunk_shape[idx] = cube_size;
	}
	cout << "generated chunk shape: " << endl;
	for (auto &c : chunk_shape)
		cout << c << " ";
	cout << endl;
	return chunk_shape;
}

vector<vector<hsize_t>> Chunking(vector<hsize_t> dset_shape, int data_size,
	vector<vector<vector<hsize_t>>> &queries, SPlan sp) {
	int idx = static_cast<int>(sp);
	const vector<int> chunk_sizes = CHUNK_SIZES[idx];
	int ndims = dset_shape.size();
	
	int dset_size = data_size;
	for (auto &d : dset_shape)
		dset_size *= d;

	vector<vector<size_t>> final_shapes;
	vector<hsize_t> chunk_shape;
	int re = -1;

	for (auto &chunk_size: chunk_sizes) {
		if (chunk_size * 1024 * 1024 > dset_size)
			continue;
		int cur = 0;
		chunk_shape = GenerateChunkShape(dset_shape, data_size, chunk_size);

		for (auto & range: queries) {
			int nchunks = 1;
			for (int i = 0; i < ndims; i++)
				nchunks *= range[i][1] / chunk_shape[i] - range[i][0] / chunk_shape[i] + 1;
			if (nchunks >= REQUEST_RANGE[idx][0] && nchunks <= REQUEST_RANGE[idx][1]) {
				cur++;
			}
		}
		Logger::log("SP: ", sp);
		Logger::log("chunk_size: ", chunk_size);
		Logger::log("satisfy: ", cur);
		if (cur == re)
			final_shapes.push_back(chunk_shape);
		else if (cur > re) {
			re = cur;
			final_shapes.clear();
			final_shapes.push_back(chunk_shape);
		}

	}
	return final_shapes;
}

vector<hsize_t> FinalDecision(vector<hsize_t> dset_shape, hid_t type_id,
	vector<vector<vector<hsize_t>>> &queries, SPlan *final_sp) {

	int data_size = H5Tget_size(type_id);
	int ndims = dset_shape.size();
	// dummy property for DSET_OBJ
	string name = "";
	string uri = "";
	string bucket_name = "";
	S3Client *client = NULL;

	// measurement
	vector<hsize_t> final_shape;
	double final_cost = DBL_MAX;

	for (SPlan sp: {SPlan::AZURE_BLOB, SPlan::GOOGLE, SPlan::S3}) {
		int idx = static_cast<int>(sp);
		vector<vector<hsize_t>> chunk_shape_cand = Chunking(dset_shape, data_size, queries, sp);
		for (auto& chunk_shape : chunk_shape_cand) {
			int nchunks = 1;
			for (int i = 0; i < ndims; i++)
				nchunks *= (dset_shape[i] - 1) / chunk_shape[i] + 1;
			vector<FileFormat> formats(nchunks, binary);
			vector<int> n_bits(nchunks, 0);

			S3VLDatasetObj *dset_obj = new S3VLDatasetObj(name, uri, type_id, ndims, dset_shape, chunk_shape, formats, n_bits, nchunks, client, bucket_name);

			double cur_cost = 0;
			for (auto & range: queries) {
				vector<S3VLChunkObj*> chunk_objs = dset_obj->generateChunks(range);
				double c;
				QueryProcess(chunk_objs, sp, &c);
				cur_cost += c;
			}
			Logger::log("SP: ", sp);
			Logger::log("Cost: ", cur_cost);
			if (cur_cost < final_cost) {
				final_cost = cur_cost;
				final_shape = chunk_shape;
				*final_sp = sp;
			}
		}
	}
	return final_shape;
}


// executable
int main(int argc, char const *argv[])
{
	string line;
	string input_file = argv[1];
	ifstream input(input_file);
	int ndims;

	input >> ndims;
	hid_t dtype = H5T_NATIVE_CHAR;

	vector<hsize_t> dset_shape;
	getline(input, line);

	for (int i = 0; i < ndims; i++) {
		int tmp;
		input >> tmp;
		dset_shape.push_back(tmp);
	}
	if (ndims > 1)
		swap(dset_shape[0], dset_shape[1]);
	getline(input, line);

	vector<vector<vector<hsize_t>>> queries;
	while(getline(input, line)) {
		istringstream iss(line);
		vector<vector<hsize_t>> q;
		for (int i = 0; i < ndims; i++) {
			size_t l, r;
			iss >> l;
			iss >> r;
			q.push_back({l, r});
		}
		if (ndims > 1)
			swap(q[0],q[1]);
		queries.push_back(q);
	}
#ifdef LOG_ENABLE
	cout << "ndims: " << ndims << endl;;
	cout << "dataset shape: ";
	for (int i = 0; i < ndims; i++)
		cout << dset_shape[i] << " ";
	cout << endl;
	for (auto &q: queries) {
		for (int i = 0; i < ndims; i++)
			cout << "[" << q[i][0] << ", " << q[i][1] << "]";
		cout << endl;
	}
#endif
	SPlan final_sp;
	vector<hsize_t> chunk_shape = FinalDecision(dset_shape, dtype, queries, &final_sp);
	cout << "final decision: " << endl;
	for (int i = 0; i < ndims; i++)
		cout << chunk_shape[i] << " ";
	cout << endl;
	cout << "SP: " << final_sp << endl;
	return 0;
}

