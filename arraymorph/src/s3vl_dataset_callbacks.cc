#include "s3vl_dataset_callbacks.h"
#include "logger.h"
#include <cstring>
#include <aws/core/utils/threading/Executor.h>
#include <assert.h>
#include <algorithm>
#include <vector>
#include "constants.h"
using namespace std;

const hid_t get_native_type(hid_t tid) {
	hid_t native_list[] = {
		H5T_NATIVE_CHAR,
		H5T_NATIVE_SHORT,
		H5T_NATIVE_INT,
		H5T_NATIVE_LONG,
		H5T_NATIVE_LLONG,
		H5T_NATIVE_UCHAR,
		H5T_NATIVE_USHORT,
		H5T_NATIVE_UINT,
		H5T_NATIVE_ULONG,
		H5T_NATIVE_ULLONG,
		H5T_NATIVE_FLOAT,
		H5T_NATIVE_DOUBLE,
		H5T_NATIVE_LDOUBLE,
		H5T_NATIVE_B8,
		H5T_NATIVE_B16,
		H5T_NATIVE_B32,
		H5T_NATIVE_B64
	};
	hid_t tmp = H5Tget_native_type(tid, H5T_DIR_DEFAULT);
	for (hid_t t: native_list) {
		if (H5Tequal(t, tmp))
			return t;
	}
	return FAIL;

}

void* S3VLDatasetCallbacks::S3VL_dataset_create(void *obj, const H5VL_loc_params_t *loc_params, const char *name, 
			hid_t lcpl_id, hid_t type_id, hid_t space_id, hid_t dcpl_id, hid_t dapl_id, hid_t dxpl_id, void **req) {
	Logger::log("------ Create Dataset: ", name);
	hid_t new_tid = get_native_type(type_id);
	if (new_tid < 0) {
		Logger::log("------ Unsupported data type");
		return NULL;
	}
	S3VLFileObj *file_obj = (S3VLFileObj*)obj;
	string uri = file_obj->name + "/" + name;

	int ndims = H5Sget_simple_extent_ndims(space_id);

	hsize_t dims[ndims];
	hsize_t chunk_dims[ndims];
	hsize_t max_dims[ndims];

	H5Sget_simple_extent_dims(space_id, dims, max_dims);

	H5D_layout_t layout = H5Pget_layout(dcpl_id);
    
	if (layout == H5D_CHUNKED) {
		H5Pget_chunk(dcpl_id, ndims, chunk_dims);
	}
	else {
		memcpy(chunk_dims, dims, sizeof(hsize_t) * ndims);
	}
	int nchunks = 1;
	for (int i = 0; i < ndims; i++)
		nchunks *= (dims[i] - 1) / chunk_dims[i] + 1;
	// initially store chunk as binary
	vector<FileFormat> formats(nchunks, binary);
	// initially fill with 0
	vector<hsize_t> shape(dims, dims + ndims);
	vector<hsize_t> chunk_shape(chunk_dims, chunk_dims + ndims);
    vector<int> n_bits(nchunks, 0);
    if (ndims >= 2) {
		swap(shape[0], shape[1]);
		swap(chunk_shape[0], chunk_shape[1]);
	}
	string bucket_name = getenv("BUCKET_NAME");
	// aws connection
	Aws::S3::S3Client *client;
	if (SP == SPlan::S3) {
		string access_key = getenv("AWS_ACCESS_KEY_ID");
		string secret_key = getenv("AWS_SECRET_ACCESS_KEY");
		Aws::Auth::AWSCredentials cred(access_key, secret_key);
		ClientConfiguration config;
		//config.scheme = Scheme::HTTP;
		config.maxConnections = s3Connections;
		config.requestTimeoutMs = requestTimeoutMs;
		config.connectTimeoutMs = connectTimeoutMs;
#ifdef POOLEXECUTOR
    	config.executor = Aws::MakeShared<Aws::Utils::Threading::PooledThreadExecutor>("test", poolSize);
#endif
		Logger::log("------ Create Client: maxConnections=", config.maxConnections);
    	client = new S3Client(cred, config);
	}
	else{
		client = NULL;
	}
	

	S3VLDatasetObj *ret_obj = new S3VLDatasetObj(name, uri, new_tid, ndims, shape, chunk_shape, formats, n_bits, nchunks, client, bucket_name);
	ret_obj->is_modified = true;
	Logger::log("------ Create Metadata:");
	Logger::log(ret_obj->to_string());
	return (void*)ret_obj;

}
void* S3VLDatasetCallbacks::S3VL_dataset_open(void *obj, const H5VL_loc_params_t *loc_params, const char *name, hid_t dapl_id, 
			hid_t dxpl_id, void **req) {
	Logger::log("------ Open dataset ", name);
	S3VLFileObj *file_obj = (S3VLFileObj*)obj;
	string dset_uri = file_obj->name + "/" + name + "/meta";
	string bucket_name = getenv("BUCKET_NAME");
	// aws connection
	Aws::S3::S3Client *client;
	if (SP == SPlan::S3) {
		string access_key = getenv("AWS_ACCESS_KEY_ID");
		string secret_key = getenv("AWS_SECRET_ACCESS_KEY");
		Aws::Auth::AWSCredentials cred(access_key, secret_key);
		ClientConfiguration config;
		//config.scheme = Scheme::HTTP;
		config.maxConnections = s3Connections;
		config.requestTimeoutMs = requestTimeoutMs;
		config.connectTimeoutMs = connectTimeoutMs;
#ifdef POOLEXECUTOR
    	config.executor = Aws::MakeShared<Aws::Utils::Threading::PooledThreadExecutor>("test", poolSize);
#endif
		Logger::log("------ Create Client: maxConnections=", config.maxConnections);
		client = new S3Client(cred, config);
	}
	else {
		client = NULL;
	}

// azure connection

	S3VLDatasetObj *dset_obj = S3VLDatasetObj::getDatasetObj(client, bucket_name, dset_uri);
	hid_t type_id = dset_obj->dtype;
	return (void*)dset_obj;
}
herr_t S3VLDatasetCallbacks::S3VL_dataset_read(size_t count, void **dset, hid_t *mem_type_id, hid_t *mem_space_id, hid_t *file_space_id, 
			hid_t plist_id, void **buf, void **req) {
	S3VLDatasetObj *dset_obj = (S3VLDatasetObj*)(dset[0]);
	Logger::log("------ Read dataset");
	Logger::log("------ Read dataset ", dset_obj->uri);
	// string lower_range = getenv("LOWER_RANGE");
	// string upper_range = getenv("UPPER_RANGE");
	// cout << lower_range << " " << upper_range << endl;
	if (dset_obj->read(*mem_space_id, *file_space_id, buf[0])) {
		Logger::log("read successfully");
		return SUCCESS;
	}
	Logger::log("read failed");
	return FAIL;
}
herr_t S3VLDatasetCallbacks::S3VL_dataset_write(size_t count, void **dset, hid_t *mem_type_id, hid_t *mem_space_id, hid_t *file_space_id, 
			hid_t plist_id, const void **buf, void **req) {
	// TODO: update
	S3VLDatasetObj *dset_obj = (S3VLDatasetObj*)(dset[0]);
	Logger::log("------ Write dataset ", dset_obj->uri);
	// vector<int> mem_space = get_range_from_dataspace(mem_space_id);
	// vector<int> file_space = get_range_from_dataspace(file_space_id);

	if (dset_obj->write(*mem_space_id, *file_space_id, buf[0])) {
		Logger::log("write successfully");
		return SUCCESS;
	}
	Logger::log("write failed");
	return FAIL;
}

herr_t S3VLDatasetCallbacks::S3VL_dataset_get(void *dset, H5VL_dataset_get_args_t *args, hid_t dxpl_id, void **req) {
	S3VLDatasetObj *dset_obj = (S3VLDatasetObj*)dset;
	Logger::log("------ Get Space dataset: ", args->op_type);
	if (args->op_type == H5VL_dataset_get_t::H5VL_DATASET_GET_DCPL) {
		hid_t dcpl_id = H5Pcreate(H5P_DATASET_CREATE);
		args->args.get_dcpl.dcpl_id = H5Pcopy(dcpl_id);
	}
	else if (args->op_type == H5VL_dataset_get_t::H5VL_DATASET_GET_SPACE) {
		hid_t space_id = H5Screate_simple(dset_obj->ndims, dset_obj->shape.data(), NULL);
		args->args.get_space.space_id = space_id;
	}
	else if (args->op_type == H5VL_dataset_get_t::H5VL_DATASET_GET_TYPE) {
		hid_t type_id = H5Tcopy(dset_obj->dtype);
		args->args.get_type.type_id = type_id;
	}
	return SUCCESS;
}
herr_t S3VLDatasetCallbacks::S3VL_dataset_close(void *dset, hid_t dxpl_id, void **req) {
	// TODO: update metadata
	Logger::log("------ Close dataset");
	S3VLDatasetObj *dset_obj = (S3VLDatasetObj*)dset;
	if (dset_obj->is_modified)
		dset_obj->upload();
	
	delete dset_obj;
	return SUCCESS;
}

void* S3VLDatasetCallbacks::S3VL_obj_open(void* obj, const H5VL_loc_params_t *loc_params, H5I_type_t *opened_type, hid_t dxpl_id, void **req) {
	Logger::log("------ Open object");
	*opened_type = H5I_type_t::H5I_DATASET;
	return S3VLDatasetCallbacks::S3VL_dataset_open(obj, loc_params, loc_params->loc_data.loc_by_name.name, dxpl_id, dxpl_id, req);
}

herr_t S3VLDatasetCallbacks::S3VL_obj_get(void *obj, const H5VL_loc_params_t *loc_params, H5VL_object_get_args_t *args, hid_t dxpl_id, void **req) {
	Logger::log("------ Object get");
	args->op_type = H5VL_object_get_t::H5VL_OBJECT_GET_NAME;
	auto by_name = args->args.get_name;
	by_name.buf_size = 4;
	string name = "test";
	by_name.buf = new char[5];
	strcpy(by_name.buf, name.c_str());
	size_t *ptr;
	*ptr = 4;
	by_name.name_len = ptr;
	return SUCCESS;
}	
void* S3VLDatasetCallbacks::S3VL_wrap_object(void *obj, H5I_type_t obj_type, void* wrap_ctx) {
	Logger::log("------ Wrap object");
	return obj;
}

void* S3VLDatasetCallbacks::S3VL_get_object(const void *obj) {
	Logger::log("------ Get object");
	return NULL;
}

herr_t S3VLDatasetCallbacks::S3VL_dataset_specific(void *obj, H5VL_dataset_specific_args_t *args, hid_t dxpl_id, void **req ) {
	Logger::log("------ Specific dataset: ", args->op_type);
	return SUCCESS;
}