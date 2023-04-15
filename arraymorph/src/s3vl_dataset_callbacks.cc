#include "s3vl_dataset_callbacks.h"
#include "logger.h"
#include <cstring>
#include <aws/core/utils/threading/Executor.h>
#include <assert.h>
#include <algorithm>
#include <vector>
using namespace std;
void* S3VLDatasetCallbacks::S3VL_dataset_create(void *obj, const H5VL_loc_params_t *loc_params, const char *name, 
			hid_t lcpl_id, hid_t type_id, hid_t space_id, hid_t dcpl_id, hid_t dapl_id, hid_t dxpl_id, void **req) {
	Logger::log("------ Create Dataset: ", name);
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
	// aws connection
	string access_key = getenv("AWS_ACCESS_KEY_ID");
    string secret_key = getenv("AWS_SECRET_ACCESS_KEY");
    Aws::Auth::AWSCredentials cred(access_key, secret_key);
    string bucket_name = getenv("BUCKET_NAME");
    ClientConfiguration config;
    //config.scheme = Scheme::HTTP;
    config.maxConnections = s3Connections;
    config.requestTimeoutMs = requestTimeoutMs;
    config.connectTimeoutMs = connectTimeoutMs;
#ifdef POOLEXECUTOR
    config.executor = Aws::MakeShared<Aws::Utils::Threading::PooledThreadExecutor>("test", poolSize);
#endif
	Logger::log("------ Create Client: maxConnections=", config.maxConnections);
    Aws::S3::S3Client *client = new S3Client(cred, config);
	

	S3VLDatasetObj *ret_obj = new S3VLDatasetObj(name, uri, type_id, ndims, shape, chunk_shape, formats, n_bits, nchunks, client, bucket_name);
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
	// aws connection
	string access_key = getenv("AWS_ACCESS_KEY_ID");
    string secret_key = getenv("AWS_SECRET_ACCESS_KEY");
    Aws::Auth::AWSCredentials cred(access_key, secret_key);
    string bucket_name = getenv("BUCKET_NAME");
    ClientConfiguration config;
    //config.scheme = Scheme::HTTP;
    config.maxConnections = s3Connections;
    config.requestTimeoutMs = requestTimeoutMs;
    config.connectTimeoutMs = connectTimeoutMs;
#ifdef POOLEXECUTOR
    config.executor = Aws::MakeShared<Aws::Utils::Threading::PooledThreadExecutor>("test", poolSize);
#endif
    Logger::log("------ Create Client: maxConnections=", config.maxConnections);
    Aws::S3::S3Client *client = new S3Client(cred, config);

// azure connection

	S3VLDatasetObj *dset_obj = S3VLDatasetObj::getDatasetObj(client, bucket_name, dset_uri);
	return (void*)dset_obj;
}
herr_t S3VLDatasetCallbacks::S3VL_dataset_read(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, 
			hid_t plist_id, void *buf, void **req) {
	S3VLDatasetObj *dset_obj = (S3VLDatasetObj*)dset;
	Logger::log("------ Read dataset ", dset_obj->uri);
	// string lower_range = getenv("LOWER_RANGE");
	// string upper_range = getenv("UPPER_RANGE");
	// cout << lower_range << " " << upper_range << endl;
	if (dset_obj->read(mem_space_id, file_space_id, buf)) {
		Logger::log("read successfully");
		return SUCCESS;
	}
	Logger::log("read failed");
	return FAIL;
}
herr_t S3VLDatasetCallbacks::S3VL_dataset_write(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, 
			hid_t plist_id, const void *buf, void **req) {
	// TODO: update
	S3VLDatasetObj *dset_obj = (S3VLDatasetObj*)dset;
	Logger::log("------ Write dataset ", dset_obj->uri);
	// vector<int> mem_space = get_range_from_dataspace(mem_space_id);
	// vector<int> file_space = get_range_from_dataspace(file_space_id);

	if (dset_obj->write(mem_space_id, file_space_id, buf)) {
		Logger::log("write successfully");
		return SUCCESS;
	}
	Logger::log("write failed");
	return FAIL;
}

herr_t S3VLDatasetCallbacks::S3VL_dataset_get(void *dset, H5VL_dataset_get_args_t *args, hid_t dxpl_id, void **req) {
	S3VLDatasetObj *dset_obj = (S3VLDatasetObj*)dset;
	Logger::log("------ Get Space dataset");
	hid_t space_id = H5Screate_simple(dset_obj->ndims, dset_obj->shape.data(), NULL);
	args->args.get_space.space_id = space_id;
	return SUCCESS;
}
herr_t S3VLDatasetCallbacks::S3VL_dataset_close(void *dset, hid_t dxpl_id, void **req) {
	// TODO: update metadata
	S3VLDatasetObj *dset_obj = (S3VLDatasetObj*)dset;
	if (dset_obj->is_modified)
		dset_obj->upload();
	Logger::log("------ Close dataset");
	delete dset_obj;
	return SUCCESS;
}
