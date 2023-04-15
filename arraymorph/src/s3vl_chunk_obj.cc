#include "s3vl_chunk_obj.h"
#include "utils.h"
#include <sstream>
#include <algorithm>
#include <assert.h>
S3VLChunkObj::S3VLChunkObj(string name, FileFormat format, hid_t dtype, vector<vector<hsize_t>> ranges,
			vector<hsize_t> shape, int n_bits, vector<hsize_t> return_offsets) {
	this->uri = name;
	this->format = format;
	this->dtype = dtype;
	this->data_size = H5Tget_size(dtype);
	this->ranges = ranges;
	this->shape = shape;
	this->ndims = shape.size();
	this->n_bits = n_bits;
	this->global_offsets = return_offsets;
	assert(this->ndims = ranges.size());

	vector<hsize_t>reduc_per_dim(this->ndims);
	reduc_per_dim[0] = 1;
	for (int i = 1; i < this->ndims; i++)
		reduc_per_dim[i] = reduc_per_dim[i - 1] * shape[i - 1];
	this->reduc_per_dim = reduc_per_dim;
	this->local_offsets = calSerialOffsets(ranges, shape);
	this->size = reduc_per_dim[ndims - 1] * shape[ndims - 1] * data_size;
    hsize_t sr = 1;
    for (int i = 0; i < ndims; i++)
		sr *= ranges[i][1] - ranges[i][0] + 1;
	this->required_size = sr * data_size;
}

bool S3VLChunkObj::checkFullWrite() {
	for (int i = 0; i < ndims; i++) {
		if (ranges[i][0] != 0 || ranges[i][1] != shape[i] - 1)
			return false;
	}
	return true;
}

// string Query::toSelect() const{

// 	int incr = 1;
// 	int n = n_bits;
// 	while(n--)
// 		incr *= 10;
// 	string proj, pred_cast;
// 	if (data_type == int32) 
// 		proj = "CAST(s.v as integer)%" + std::to_string(incr);
// 	else if (data_type == float64)
// 		proj = "CAST(s.v as float) - CAST(CAST(s.v as float) as integer)/" + 
// 		std::to_string(incr) + "*" + std::to_string(incr);
// 	if (data_type == int32)
// 		pred_cast = "(CAST(s.v as integer)/" + std::to_string(incr) + ")";
// 	else
// 		pred_cast = "(CAST(CAST(s.v as float) as integer)/" + std::to_string(incr) + ")";
// 	string pred;

// 	for (int i = dims - 1; i >= 0; i--) {
// 		string cur = pred_cast + "/" + std::to_string(reduc_per_dim[i]);
// 		pred += "((" + cur + " BETWEEN " + std::to_string(ranges[i][0]) + " AND " +
// 				std::to_string(ranges[i][1]) + ") OR (" + cur + " BETWEEN " 
// 				+ std::to_string(-ranges[i][1]) + " AND " +std::to_string(-ranges[i][0]) + ")) AND ";
// 		pred_cast += "%" + std::to_string(reduc_per_dim[i]);
// 	}

// 	pred = pred.substr(0, pred.size() - 5);

// 	string expr = "SELECT " + proj + " FROM s3object s WHERE " + pred;
// 	return expr;
// }

// vector<vector<uint64_t>> Query::toByteRanges() {
// 	vector<vector<uint64_t>> re;
// 	vector<int> offsets = calSerialOffsets(ranges, shape);
// 	for (auto &offset: offsets) {
// 		uint64_t start = (offset + ranges[0][0]) * data_size;
// 		uint64_t end = (offset + ranges[0][1] + 1) * data_size - 1;
// 		re.push_back({start, end});
// 	}
// 	return re;
// }

string S3VLChunkObj::to_string() {
	stringstream ss;
	ss << uri << " " << dtype << " " << format << " " << ndims << endl;
	for (auto &s : shape)
		ss << s << " ";
	ss << endl;
	for (int i = 0; i < ndims; i++) {
		ss << shape[i] << " [" << ranges[i][0] << ", " << ranges[i][1] << "]" << endl; 
	}
	ss << n_bits << endl;
	return ss.str();
}
