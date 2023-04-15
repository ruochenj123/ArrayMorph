#ifndef S3VL_DATASET_OBJ
#define S3VL_DATASET_OBJ
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
// #include "query.h"
#include "constants.h"
#include <hdf5.h>
#include "operators.h"
#include "query_processor.h"
using namespace std;

class S3VLDatasetObj
{
public:
	S3VLDatasetObj(string name, string uri, hid_t dtype, int ndims, vector<hsize_t> shape, vector<hsize_t> chunk_shape,
			vector<FileFormat> formats, vector<int> n_bits, int chunk_num, S3Client *client, string bucket_name);
	~S3VLDatasetObj(){};

	static S3VLDatasetObj* getDatasetObj(S3Client *client, string bucket_name, string uri);
	static S3VLDatasetObj* getDatasetObj(S3Client *client, string bucket_name, char* buffer);
	char* toBuffer(int *length);
	vector<S3VLChunkObj*> generateChunks(vector<vector<hsize_t>> ranges);
	string to_string();
	vector<hsize_t> getChunkOffsets(int chunk_idx);
	vector<vector<hsize_t>> getChunkRanges(int chunk_idx);
	vector<vector<hsize_t>> selectionFromSpace(hid_t space_id);
	// QPlan getQueryPlan(FileFormat format, vector<vector<hsize_t>> ranges);

	void upload();
	herr_t write(hid_t mem_space_id, hid_t file_space_id, const void* buf);
	herr_t read(hid_t mem_space_id, hid_t file_space_id, void* buf);

	string name;
	string uri;
	hid_t dtype;
	hsize_t data_size;
	vector<hsize_t> shape;
	vector<hsize_t> chunk_shape;
	vector<FileFormat> formats;
	vector<hsize_t> num_per_dim;
	vector<hsize_t> reduc_per_dim;
	vector<int> n_bits;
	int chunk_num;
	int ndims;
	hsize_t element_per_chunk;
	bool is_modified;
	S3Client *s3_client;
	string bucket_name;
	gcs::Client *gc_client;
};


#endif