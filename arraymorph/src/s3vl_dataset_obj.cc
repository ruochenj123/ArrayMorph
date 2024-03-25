#include "s3vl_dataset_obj.h"
#include "utils.h"
#include "logger.h"
#include <algorithm>
#include <sstream>
#include <assert.h>
#include <fstream>
#include <cstring>
#include <chrono>
#include <thread>
#include <sys/time.h>
atomic<int> finish;
uint64_t transfer_size;
int s3_req_num, azure_req_num, gc_req_num, lambda_num, merge_num, multi_fetch_num, get_num;

S3VLDatasetObj::S3VLDatasetObj(string name, string uri, hid_t dtype, int ndims, vector<hsize_t> shape, vector<hsize_t> chunk_shape, 
		vector<FileFormat> formats, vector<int> n_bits, int chunk_num, S3Client *client, string bucket_name 
		) {

	this->name = name;
	this->uri = uri;
	this->dtype = dtype;
	this->ndims = ndims;
	this->shape = shape;
	this->chunk_shape = chunk_shape;
	this->data_size = H5Tget_size(this->dtype);
	this->formats = formats;
	this->chunk_num = chunk_num;
	assert(chunk_num == formats.size());
	vector<hsize_t> num_per_dim(ndims);
	vector<hsize_t> reduc_per_dim(ndims);
	reduc_per_dim[0] = 1;
	for (int i = 0; i < ndims; i++)
		num_per_dim[i] = (this->shape[i] - 1) / this->chunk_shape[i] + 1;
	for (int i = 1; i < ndims; i++)
		reduc_per_dim[i] = reduc_per_dim[i - 1] * num_per_dim[i - 1];
	this->num_per_dim = num_per_dim;
	this->reduc_per_dim = reduc_per_dim;
	this->n_bits = n_bits;
	assert(reduc_per_dim[ndims - 1] * num_per_dim[ndims - 1] == chunk_num);

	hsize_t element_per_chunk = 1;
	for (auto &s: this->chunk_shape)
		element_per_chunk *= s;
	this->element_per_chunk = element_per_chunk;
    this->is_modified = false;

    // AWS connection
    this->bucket_name = bucket_name;
    this->s3_client = client;

    // GC connection
	if (SP == GOOGLE) {
		string gc_connection_file = getenv("GOOGLE_CLOUD_STORAGE_JSON");
		auto is = ifstream(gc_connection_file);
		auto json_string =
				std::string(std::istreambuf_iterator<char>(is.rdbuf()), {});
		auto credentials =
				google::cloud::MakeServiceAccountCredentials(json_string);
		this->gc_client = new gcs::Client(
				google::cloud::Options{}.set<google::cloud::UnifiedCredentialsOption>(
				credentials).set<gcs::ConnectionPoolSizeOption>(gcConnections));
	}
	else {
		this->gc_client = NULL;
	}
}

vector<hsize_t> S3VLDatasetObj::getChunkOffsets(int chunk_idx) {
	vector<hsize_t> idx_per_dim(ndims);
	int tmp = chunk_idx;
	for (int i = ndims - 1; i >= 0; i--) {
		idx_per_dim[i] = tmp / reduc_per_dim[i] * chunk_shape[i];
		tmp %= reduc_per_dim[i];
	}
	return idx_per_dim;
}

vector<vector<hsize_t>> S3VLDatasetObj::getChunkRanges(int chunk_idx) {
	vector<hsize_t> offsets_per_dim = getChunkOffsets(chunk_idx);
	vector<vector<hsize_t>> re(ndims);
	for (int i = 0; i < ndims; i++)
		re[i] = {offsets_per_dim[i], offsets_per_dim[i] + chunk_shape[i] - 1};
	return re;
}

vector<vector<hsize_t>> S3VLDatasetObj::selectionFromSpace(hid_t space_id) {
	hsize_t start[ndims];
	hsize_t end[ndims];
	vector<vector<hsize_t>> ranges(ndims);
	H5Sget_select_bounds(space_id, start, end);
	for (int i = 0; i < ndims; i++)
		ranges[i] = {start[i], end[i]};
	if (ndims >= 2)
		swap(ranges[0], ranges[1]);
	return ranges;
}

void processGC(vector<S3VLChunkObj*> &chunk_objs, vector<CPlan> &gc_plans, 
	vector<vector<vector<hsize_t>>> &mappings, void* buf, gcs::Client *gc_client, 
	string bucket_name) {

    const char* lambda_url = getenv("GC_LAMBDA_URL");

	gc_req_num = gc_plans.size();
	vector<thread> threads;
	threads.reserve(gc_thread_num);
	int gc_idx = 0;
	while(gc_idx < gc_req_num) {
		for (int idx = gc_idx; idx < min(gc_req_num, gc_idx + gc_thread_num); idx++) {
			int i = gc_plans[idx].chunk_id;
			if (gc_plans[idx].qp == GET) {
				shared_ptr<AsyncCallerContext> context(new AsyncReadInput(buf, mappings[i]));
				transfer_size += chunk_objs[i]->size;
				thread gc_th(Operators::GCGet, gc_client, bucket_name, chunk_objs[i]->uri, context);
				threads.push_back(move(gc_th));
			}
			else if (gc_plans[idx].qp == MERGE) {
				uint64_t beg = mappings[i][0][0];
				uint64_t end = mappings[i].back()[0] + mappings[i].back()[2] - 1;
				for (auto &m : mappings[i])
					m[0] -= beg;
            	transfer_size += end - beg + 1;
				shared_ptr<AsyncCallerContext> context(
					new AsyncReadInput(buf, mappings[i]));
				thread gc_th(Operators::GCGetRange, gc_client, bucket_name, chunk_objs[i]->uri, beg, end, context);
				threads.push_back(move(gc_th));
			}
			else if(gc_plans[idx].qp == GET_BYTE_RANGE) {
				int rid = gc_plans[idx].row_id;
				auto m = mappings[i][rid];
				uint64_t beg = m[0];
				uint64_t end = beg + m[2] - 1;
                transfer_size += m[2];
				shared_ptr<AsyncCallerContext> context(
					new AsyncReadInput(buf, {{0, m[1], m[2]}}));
				thread gc_th(Operators::GCGetRange, gc_client, bucket_name, chunk_objs[i]->uri, beg, end, context);
	        	threads.push_back(move(gc_th));
			}
			else {
				// process
				string query = gc_plans[idx].lambda_query;
				uint64_t rsize = mappings[i][0][2];
            	transfer_size += rsize * mappings[i].size();
				for (int j = 0; j < mappings[i].size(); j++)
					mappings[i][j][0] = j * rsize;
				shared_ptr<AsyncCallerContext> context(new AsyncReadInput(buf, mappings[i], 1));
				thread gc_th(Operators::HttpTrigger, lambda_url, query, context);
				threads.push_back(move(gc_th));
			}

		}
		gc_idx = min(gc_req_num, gc_idx + gc_thread_num);
		for (auto &t: threads)
			t.join();
		threads.clear();
        // cout << "gc_idx: " << gc_idx << endl;
	}
}

void processAzure(vector<S3VLChunkObj*> &chunk_objs, vector<CPlan> &azure_plans, 
	vector<vector<vector<hsize_t>>> &mappings,
	void* buf, BlobContainerClient &lclient, string bucket_name) {
	azure_req_num = azure_plans.size();
	const char* lambda_url = getenv("AZURE_LAMBDA_URL");
	vector<thread> threads;
	threads.reserve(azure_thread_num);
	int azure_idx = 0;
	while(azure_idx < azure_req_num) {
		for (int idx = azure_idx; idx < min(azure_req_num, azure_idx + azure_thread_num); idx++) {
			int i = azure_plans[idx].chunk_id;
			if (azure_plans[idx].qp == GET) {
				transfer_size += chunk_objs[i]->size;
				shared_ptr<AsyncCallerContext> context(new AsyncReadInput(buf, mappings[i]));
				thread azure_th(Operators::AzureGet, std::ref(lclient), chunk_objs[i]->uri, context);
				threads.push_back(move(azure_th));
			}
			else if (azure_plans[idx].qp == MERGE) {
				uint64_t beg = mappings[i][0][0];
				uint64_t end = mappings[i].back()[0] + mappings[i].back()[2] - 1;
				for (auto &m : mappings[i])
					m[0] -= beg;
            	transfer_size += end - beg + 1;
				shared_ptr<AsyncCallerContext> context(
					new AsyncReadInput(buf, mappings[i]));
				thread azure_th(Operators::AzureGetRange, std::ref(lclient), chunk_objs[i]->uri, beg, end, context);
				threads.push_back(move(azure_th));
			}
			else if(azure_plans[idx].qp == GET_BYTE_RANGE) {
				int rid = azure_plans[idx].row_id;
				auto m = mappings[i][rid];
				uint64_t beg = m[0];
				uint64_t end = beg + m[2] - 1;
                transfer_size += m[2];
				shared_ptr<AsyncCallerContext> context(
					new AsyncReadInput(buf, {{0, m[1], m[2]}}));
				thread azure_th(Operators::AzureGetRange, std::ref(lclient), chunk_objs[i]->uri, beg, end, context);
	        	threads.push_back(move(azure_th));
			}
			else {
				// process
				string query = azure_plans[idx].lambda_query;
				uint64_t rsize = mappings[i][0][2];
            	transfer_size += rsize * mappings[i].size();
				for (int j = 0; j < mappings[i].size(); j++)
					mappings[i][j][0] = j * rsize;
				shared_ptr<AsyncCallerContext> context(new AsyncReadInput(buf, mappings[i], 1));
				thread gc_th(Operators::HttpTrigger, lambda_url, query, context);
				threads.push_back(move(gc_th));
			}
		}
		azure_idx = min(azure_req_num, azure_idx + azure_thread_num);
		for (auto &t : threads)
			t.join();
		threads.clear();
		// cout << "azure_idx: " << azure_idx << endl;
	}
}

herr_t S3VLDatasetObj::read(hid_t mem_space_id, hid_t file_space_id, void* buf) {
	const char* lambda_path = getenv("AWS_LAMBDA_ACCESS_POINT");
	// string lambda_merge_path = getenv("AWS_LAMBDA_MERGE_ACCESS_POINT");
    vector<vector<hsize_t>> ranges;
	if (file_space_id != H5S_ALL) {
		ranges = selectionFromSpace(file_space_id);
	}
	else {
		for (int i = 0;i < ndims; i++)
			ranges.push_back({0, shape[i] - 1});
	}
	
	vector<S3VLChunkObj*> chunk_objs = generateChunks(ranges);
	int num = chunk_objs.size();
	vector<vector<vector<hsize_t>>> mappings(num);

	vector<hsize_t> out_offsets;
	hsize_t out_row_size;
	if (mem_space_id == H5S_ALL) {
		// memspace == dataspace
		out_offsets = calSerialOffsets(ranges, shape);
		out_row_size = ranges[0][1] - ranges[0][0] + 1;
	}
	else {
		vector<vector<hsize_t>> out_ranges = selectionFromSpace(mem_space_id);
		hsize_t dims_out[ndims];
		H5Sget_simple_extent_dims(mem_space_id, dims_out, NULL);
		vector<hsize_t> out_shape(dims_out, dims_out + ndims);
		if (ndims >= 2)
			swap(out_shape[0], out_shape[1]);
		out_offsets = calSerialOffsets(out_ranges, out_shape);
		out_row_size = out_ranges[0][1] - out_ranges[0][0] + 1;
	}
	for (int i = 0; i < num; i++) {
		hsize_t input_row_size = chunk_objs[i]->ranges[0][1] - chunk_objs[i]->ranges[0][0] + 1;
		mappings[i] = mapHyperslab(chunk_objs[i]->local_offsets, chunk_objs[i]->global_offsets, out_offsets, 
						input_row_size, out_row_size, data_size);
	}
	finish = 0;
    transfer_size = 0;
    s3_req_num = 0;
    gc_req_num = 0;
    azure_req_num = 0;
    lambda_num = 0;
    merge_num = 0;
    multi_fetch_num = 0;
    get_num = 0;

    double cost;
    // cout << "start plan" << endl;
    struct timeval start_opt, end_opt; 
    gettimeofday(&start_opt, NULL);
    vector<CPlan> plans = QueryProcess(chunk_objs, SP, &cost);
    gettimeofday(&end_opt, NULL);
    double opt_t = (1000000 * ( end_opt.tv_sec - start_opt.tv_sec )
                        + end_opt.tv_usec -start_opt.tv_usec) /1000000.0;
    assert(plans.size() == num);
#ifdef PROFILE_ENABLE
    cout << "query processer time: " << opt_t << endl;
    cout << "chunk num:" << num << endl;
#endif
    // cout << "get plans" << endl;
#ifdef LOG_ENABLE
    Logger::log("------ Plans:");
    for (int i = 0; i < num; i++) {
    	Logger::log("chunk: ", chunk_objs[i]->uri);
    	Logger::log("plan: ", plans[i].qp);
    }
#endif
    vector<CPlan> s3_plans, azure_plans, gc_plans;

    for (int i = 0; i < num; i++) {
    	CPlan p = plans[i];
    	if (p.qp == LAMBDA)
    		lambda_num++;
    	else if (p.qp == GET)
    		get_num++;
    	else if (p.qp == MERGE)
    		merge_num++;
    	else
    		multi_fetch_num++;
    	if (SP == SPlan::S3) {
    		if (p.qp == LAMBDA)
    			p.lambda_query = chunk_objs[i]->uri + createQuery(data_size, ndims, chunk_objs[i]->shape, chunk_objs[i]->ranges);
    		s3_plans.push_back(p);
    	}
    	else {
    		if (p.qp == GET_BYTE_RANGE) {
    			for (int j = 0; j < mappings[i].size(); j++) {
    				CPlan new_p = {i, GET_BYTE_RANGE, j};
    				if (SP == SPlan::AZURE_BLOB)
    					azure_plans.push_back(p);
    				else
    					gc_plans.push_back(p);
    			}
    		}
    		else {
    			if (p.qp == LAMBDA)
    				p.lambda_query = chunk_objs[i]->uri + createQuery(data_size, ndims, chunk_objs[i]->shape, chunk_objs[i]->ranges);
    			if (SP == SPlan::AZURE_BLOB)
    				azure_plans.push_back(p);
    			else
    				gc_plans.push_back(p);
    		}
    	}
    }
#ifdef PROFILE_ENABLE
    cout << "Plans: " << endl;
    cout << "GET: " << get_num << endl;
    cout << "MERGE: " <<merge_num << endl;
    cout << "LAMBDA: " << lambda_num << endl;
    cout << "Multi-fetch: " << multi_fetch_num << endl;
#endif
    // float ratio = 1;
    // for (int i = 0; i < int(ratio * s3_plans.size()); i++)
    //     s3_plans[i].qp = MERGE;
    // for (int i = 0; i < int(ratio * gc_plans.size()); i++)
    //     gc_plans[i].qp = MERGE;
    // for (int i = 0; i < int(ratio * azure_plans.size()); i++)
    //     azure_plans[i].qp = MERGE;

    // process S3 async
    for (auto &p: s3_plans) {
    	int i = p.chunk_id;
    	if (p.qp == GET_BYTE_RANGE) {
    		for (auto &m: mappings[i]) {
				uint64_t beg = m[0];
				uint64_t end = beg + m[2] - 1;
                transfer_size += m[2];
				shared_ptr<AsyncCallerContext> context(
					new AsyncReadInput(buf, {{0, m[1], m[2]}}));
				Operators::S3GetByteRangeAsync(s3_client, bucket_name, chunk_objs[i]->uri, beg, end, context);
	        	s3_req_num++;
	        }
    	}
    	else if (p.qp == GET) {
    		shared_ptr<AsyncCallerContext> context(new AsyncReadInput(buf, mappings[i]));
			transfer_size += chunk_objs[i]->size;
			Operators::S3GetAsync(s3_client, bucket_name, chunk_objs[i]->uri, context);
			s3_req_num++;
    	}
    	else if (p.qp == MERGE) {
    		uint64_t beg = mappings[i][0][0];
			uint64_t end = mappings[i].back()[0] + mappings[i].back()[2] - 1;
			for (auto &m : mappings[i])
				m[0] -= beg;
            transfer_size += end - beg + 1;
			shared_ptr<AsyncCallerContext> context(
				new AsyncReadInput(buf, mappings[i]));
			Operators::S3GetByteRangeAsync(s3_client, bucket_name, chunk_objs[i]->uri, beg, end, context);
    		s3_req_num++;
    	}
    	else if (p.qp == LAMBDA) {
			// process
			uint64_t rsize = mappings[i][0][2];
            transfer_size += rsize * mappings[i].size();
			for (int j = 0; j < mappings[i].size(); j++)
				mappings[i][j][0] = j * rsize;
			shared_ptr<AsyncCallerContext> context(new AsyncReadInput(buf, mappings[i], 1));
			Operators::S3GetAsync(s3_client, lambda_path, p.lambda_query, context);
            s3_req_num++;
    	}
    }

    thread azure_process, gc_process;
    if (SP == AZURE_BLOB) {
		string azure_connection_string = getenv("AZURE_STORAGE_CONNECTION_STRING");
		BlobContainerClient lclient
		= BlobContainerClient::CreateFromConnectionString(azure_connection_string, bucket_name);
        azure_process = thread(processAzure, std::ref(chunk_objs), std::ref(azure_plans), 
	    std::ref(mappings), buf, std::ref(lclient), bucket_name);
	}
    else if (SP == GOOGLE)
	    gc_process = thread(processGC, std::ref(chunk_objs), std::ref(gc_plans), 
	    std::ref(mappings), buf, gc_client, bucket_name);
    if (azure_plans.size() > 0)
	    azure_process.join();
	if (gc_plans.size() > 0)
        gc_process.join();

	while (finish < s3_req_num);
#ifdef PROFILE_ENABLE
	cout << "transfer_size: " << transfer_size << endl;
	cout << "s3_req_num: " << s3_req_num << endl;
	cout << "azure_req_num: " << azure_req_num << endl;
	cout << "gc_req_num: " << gc_req_num << endl;
#endif
	return SUCCESS;
}

herr_t S3VLDatasetObj::write(hid_t mem_space_id, hid_t file_space_id, const void* buf) {
	// dummy string to initliaze a dummy azure client if Azure is not the storage platform
	string azure_connection_string = "dummy";
	if (SP == AZURE_BLOB)
		azure_connection_string = getenv("AZURE_STORAGE_CONNECTION_STRING");
	BlobContainerClient lclient
      = BlobContainerClient::CreateFromConnectionString(azure_connection_string, bucket_name);

	vector<vector<hsize_t>> ranges;
	if (file_space_id != H5S_ALL) {
		ranges = selectionFromSpace(file_space_id);
	}
	else {
		for (int i = 0;i < ndims; i++)
			ranges.push_back({0, shape[i] - 1});
	}
	
	vector<S3VLChunkObj*> chunk_objs = generateChunks(ranges);
	int num = chunk_objs.size();
	vector<vector<vector<hsize_t>>> mappings(num);
	vector<hsize_t> source_offsets;
	hsize_t source_row_size;
	if (mem_space_id == H5S_ALL) {
		// memspace == dataspace
		source_offsets = calSerialOffsets(ranges, shape);
		source_row_size = ranges[0][1] - ranges[0][0] + 1;
	}
	else {
		vector<vector<hsize_t>> source_ranges = selectionFromSpace(mem_space_id);
		hsize_t dims_source[ndims];
		H5Sget_simple_extent_dims(mem_space_id, dims_source, NULL);
		vector<hsize_t> source_shape(dims_source, dims_source + ndims);
		if (ndims >= 2)
			swap(source_shape[0], source_shape[1]);
		source_offsets = calSerialOffsets(source_ranges, source_shape);
		source_row_size = source_ranges[0][1] - source_ranges[0][0] + 1;
	}

	for (int i = 0; i < num; i++) {
		hsize_t dest_row_size = chunk_objs[i]->ranges[0][1] - chunk_objs[i]->ranges[0][0] + 1;
		mappings[i] = mapHyperslab(chunk_objs[i]->local_offsets, chunk_objs[i]->global_offsets, source_offsets, 
						dest_row_size, source_row_size, data_size);
	}

	finish = 0;
    vector<int> s3_chunks, azure_chunks, gc_chunks;
    for (int i = 0; i < num; i++) {
    	if (SP == SPlan::S3)
    		s3_chunks.push_back(i);
    	else if (SP == SPlan::AZURE_BLOB)
    		azure_chunks.push_back(i);
    	else
    		gc_chunks.push_back(i);
    }
	int azure_idx = 0, gc_idx = 0, s3_idx = 0;
	int gc_req_num = gc_chunks.size();
	int azure_req_num = azure_chunks.size();
	int s3_req_num = s3_chunks.size();
/**
	for (auto &i : s3_chunks) {
		Result re;
		re.length = chunk_objs[i]->size;
		re.data = new char[re.length];
#ifdef DUMMY_WRITE
		memset(re.data, 0, re.length);
#else
		for (auto &m: mappings[i]){
			// cout << m[0] << " " << m[1] << " " << m[2] << endl;
			memcpy(re.data + m[0], (char*)buf + m[1], m[2]);
		}
#endif
		Operators::S3PutAsync(s3_client, bucket_name, chunk_objs[i]->uri, re);
	}
**/
	int threadNum = 128;
	vector<thread> gc_threads, azure_threads, s3_threads;
	gc_threads.reserve(threadNum);
	azure_threads.reserve(threadNum);
	s3_threads.reserve(threadNum);
	while(azure_idx < azure_req_num || gc_idx < gc_req_num || s3_idx < s3_req_num) {
		for (int idx = azure_idx; idx < min(azure_req_num, azure_idx + threadNum); idx++) {
			int i = azure_chunks[idx];
			uint8_t *upload_buf = new uint8_t[chunk_objs[i]->size];
#ifdef DUMMY_WRITE
			memset(upload_buf, 0, chunk_objs[i]->size);
#else
			for (auto &m: mappings[i]){
				memcpy(upload_buf + m[0], (uint8_t*)buf + m[1], m[2]);
			}
#endif
			thread azure_th(Operators::AzurePut, std::ref(lclient), chunk_objs[i]->uri, upload_buf, chunk_objs[i]->size);
			azure_threads.push_back(move(azure_th));
		}
		azure_idx = min(azure_req_num, azure_idx + threadNum);
		for (int idx = gc_idx; idx < min(gc_req_num, gc_idx + threadNum); idx++) {
			int i = gc_chunks[idx];
			int length = chunk_objs[i]->size;
			char* upload_buf = new char[length];
#ifdef DUMMY_WRITE
			memset(upload_buf, 0, length);
#else
			for (auto &m: mappings[i]){
				// cout << m[0] << " " << m[1] << " " << m[2] << endl;
				memcpy(upload_buf + m[0], (char*)buf + m[1], m[2]);
			}
#endif
			thread gc_th(Operators::GCPut, gc_client, bucket_name, chunk_objs[i]->uri, upload_buf, length);
			gc_threads.push_back(move(gc_th));
		}
		gc_idx = min(gc_req_num, gc_idx + threadNum);

		for (int idx = s3_idx; idx < min(s3_req_num, s3_idx + threadNum); idx++) {
			int i = s3_chunks[idx];
			int length = chunk_objs[i]->size;
			char* upload_buf = new char[length];
#ifdef DUMMY_WRITE
			memset(upload_buf, 0, length);
#else
			for (auto &m: mappings[i]){
				// cout << m[0] << " " << m[1] << " " << m[2] << endl;
				// cout << "data: " << ((int*)buf)[m[1] / 4] << endl;
				memcpy(upload_buf + m[0], (char*)buf + m[1], m[2]);
			}
#endif
			thread s3_th(Operators::S3PutBuf, s3_client, bucket_name, chunk_objs[i]->uri, upload_buf, length);
			s3_threads.push_back(move(s3_th));
		}
		s3_idx = min(s3_req_num, s3_idx + threadNum);
#ifdef LOG_ENABLE
		cout << "idx: " << azure_idx << " " << gc_idx  << " " << s3_idx << endl;
#endif
		for (auto &t : azure_threads)
			t.join();
	    for (auto &t : gc_threads)
	    	t.join();
	    for (auto &t : s3_threads)
	    	t.join();
	    azure_threads.clear();
	    gc_threads.clear();
	    s3_threads.clear();
	}

/**
	for (int i = 0; i < num; i++) {
		if (chunk_objs[i]->sp == SPlan::AZURE_BLOB) {
			uint8_t *upload_buf = new uint8_t[chunk_objs[i]->size];
			for (auto &m: mappings[i]){
				memcpy(upload_buf + m[0], (uint8_t*)buf + m[1], m[2]);
			}
			Operators::AzurePut(lclient, chunk_objs[i]->uri, upload_buf, chunk_objs[i]->size);
		}
		else {
			Result re;
			re.length = chunk_objs[i]->size;
			re.data = new char[re.length];
			for (auto &m: mappings[i]){
				// cout << m[0] << " " << m[1] << " " << m[2] << endl;
				memcpy(re.data + m[0], (char*)buf + m[1], m[2]);
			}
			if (chunk_objs[i]->sp == SPlan::S3)
				Operators::S3Put(s3_client, bucket_name, chunk_objs[i]->uri, re);
			else
				Operators::GCPut(gc_client, bucket_name, chunk_objs[i]->uri, re);
		}
		s3_req_num++;
	}
**/
	// while (finish < s3_req_num);
	return SUCCESS;
}

vector<S3VLChunkObj*> S3VLDatasetObj::generateChunks(vector<vector<hsize_t>> ranges) {
	assert(ranges.size() == ndims);
	vector<int> accessed_chunks;
	// get accessed chunks
	vector<vector<hsize_t>> chunk_ranges(ndims);
	for (int i = 0; i < ndims; i++)
		chunk_ranges[i] = {ranges[i][0] / chunk_shape[i], ranges[i][1] / chunk_shape[i]};
	vector<hsize_t> chunk_offsets = calSerialOffsets(chunk_ranges, num_per_dim);
    for (int i = 0; i <= chunk_ranges[0][1] - chunk_ranges[0][0]; i++)
		for (auto &n : chunk_offsets)
			accessed_chunks.push_back(i + n);
	
	// iterate accessed chunks and generate queries
	vector<S3VLChunkObj*> chunk_objs;
	chunk_objs.reserve(accessed_chunks.size());
	Logger::log("------ # of chunks ", accessed_chunks.size());
	for (auto &c : accessed_chunks) {
		vector<hsize_t> offsets = getChunkOffsets(c);
		vector<vector<hsize_t>> local_ranges(ndims);
		for (int i = 0; i < ndims; i++) {
			hsize_t left = max(offsets[i], ranges[i][0]) - offsets[i];
			hsize_t right = min(offsets[i] + chunk_shape[i] - 1, ranges[i][1]) - offsets[i];
			local_ranges[i] = {left, right};
		}
		string chunk_uri = uri + "/" + std::to_string(c);
		FileFormat format = formats[c];
		// TODO: qplan

		// get output serial offsets for each row
		vector<vector<hsize_t>> global_ranges(ndims);
		vector<hsize_t> result_shape(ndims);
		for (int i = 0; i < ndims; i++) {
			global_ranges[i] = {local_ranges[i][0] + offsets[i] - ranges[i][0], 
								local_ranges[i][1] + offsets[i] - ranges[i][0]};
			result_shape[i] = ranges[i][1] - ranges[i][0] + 1;
		}
		vector<hsize_t> result_serial_offsets = calSerialOffsets(global_ranges, result_shape);

		
		S3VLChunkObj *chunk = new S3VLChunkObj(chunk_uri, format, dtype, local_ranges, chunk_shape, n_bits[c], result_serial_offsets);
		// Logger::log("------ Generate Chunk");
		// Logger::log(chunk->to_string());
		chunk_objs.push_back(chunk);
	}
	return chunk_objs;
}










// read/write

void S3VLDatasetObj::upload() {
	Logger::log("------ Upload metadata");
	int length;
	char* buffer = toBuffer(&length);
	string meta_name = uri + "/meta";
	Result re;
	re.data = buffer;
	re.length = length;
	if (SP == SPlan::S3)
		Operators::S3Put(s3_client, bucket_name, meta_name, re);
	else if (SP == SPlan::AZURE_BLOB) {
		string azure_connection_string = getenv("AZURE_STORAGE_CONNECTION_STRING");
		BlobContainerClient lclient
      	= BlobContainerClient::CreateFromConnectionString(azure_connection_string, bucket_name);
		Operators::AzurePut(std::ref(lclient), meta_name, (uint8_t*)buffer, length);
	}
	else
		Operators::GCPut(gc_client, bucket_name, meta_name, buffer, length);
}

S3VLDatasetObj* S3VLDatasetObj::getDatasetObj(S3Client *client, string bucket_name, string uri) {
    Result re = Operators::S3Get(client, bucket_name, uri);
	return S3VLDatasetObj::getDatasetObj(client, bucket_name, re.data);
}

char* S3VLDatasetObj::toBuffer(int *length) {
	int size = 8 + name.size() + uri.size() + sizeof(hid_t) + 4 + 2 * ndims * sizeof(hsize_t) + 4 + 2 * chunk_num * 4;
	*length = size;
	char * buffer = new char[size];
	int c = 0;
	int name_length = name.size();
	memcpy(buffer, &name_length, 4);
	c += 4;
	memcpy(buffer + c, name.c_str(), name.size());
	c += name.size();
	int uri_length = uri.size();
	memcpy(buffer + c, &uri_length, 4);
	c += 4;
	memcpy(buffer + c, uri.c_str(), uri.size());
	c += uri.size();
	memcpy(buffer + c, &dtype, sizeof(hid_t));
	c += sizeof(hid_t);
	memcpy(buffer + c, &ndims, 4);
	c += 4;
	memcpy(buffer + c, shape.data(), sizeof(hsize_t) * ndims);
	c += sizeof(hsize_t) * ndims;
	memcpy(buffer + c, chunk_shape.data(), sizeof(hsize_t) * ndims);
	c += sizeof(hsize_t) * ndims;
	memcpy(buffer + c, &chunk_num, 4);
	c += 4;
	memcpy(buffer + c, n_bits.data(), 4 * chunk_num);
	c += 4 * chunk_num;
	memcpy(buffer + c, formats.data(), 4 * chunk_num);
	return buffer;
}

S3VLDatasetObj* S3VLDatasetObj::getDatasetObj(S3Client *client, string bucket_name, char* buffer) {
	string name, uri;
	hid_t dtype;
	int c = 0;
	int name_length = *(int*)buffer;
	c += 4;
	name.assign(buffer + c, name_length);	// name
	c += name_length;
	int uri_length = *(int*)(buffer + c);
	c += 4;
	uri.assign(buffer + c, uri_length);    // uri
	c += uri_length;
	dtype = (*(hid_t*)(buffer + c));	// data type
	c += sizeof(hid_t);
	int ndims = *(int*)(buffer + c);	//ndims
	c += 4;
	vector<hsize_t> shape(ndims);	// shape
	memcpy(shape.data(), (hsize_t*)(buffer + c), sizeof(hsize_t) * ndims);
	c += sizeof(hsize_t) * ndims;
	vector<hsize_t> chunk_shape(ndims);	// chunk_shape
	memcpy(chunk_shape.data(), (hsize_t*)(buffer + c), sizeof(hsize_t) * ndims);
	c += sizeof(hsize_t) * ndims;
	int chunk_num = *(int*)(buffer + c);	// chunk num
	c += 4;
	vector<int> n_bits(chunk_num);
	memcpy(n_bits.data(), (int*)(buffer + c), chunk_num * 4);	// nbits
	c += 4 * chunk_num;
	vector<FileFormat> formats(chunk_num);
	for (int i = 0; i < chunk_num; i++) {
		int f = *(int*)(buffer + c);
		c += 4;
		// formats[i] = static_cast<FileFormat>(f);
        formats[i] = binary;
	}
	return  new S3VLDatasetObj(name, uri, dtype, ndims, shape, chunk_shape, formats, n_bits, chunk_num, client, bucket_name);
}

string S3VLDatasetObj::to_string() {
	stringstream ss;
	ss << name << " " << uri << endl;
	ss << dtype << " " << ndims << endl;
	ss << chunk_num << " " << element_per_chunk << endl;;
	for (int i = 0; i < ndims; i++) {
		ss << shape[i] << " ";
	}
	ss << endl;
	for (int i = 0; i < ndims; i++) {
		ss << chunk_shape[i] << " ";
	}
	ss << endl;
	for (int i = 0; i < ndims; i++) {
		ss << num_per_dim[i] << " ";
	}
	ss << endl;
	for (int i = 0; i < ndims; i++) {
		ss << reduc_per_dim[i] << " ";
	}
	ss << endl;
	for (auto &f: formats)
		ss << f;
	ss << endl;
	for (auto &n: n_bits)
		ss << n;
	ss << endl;
	return ss.str();
}
