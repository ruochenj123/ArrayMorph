#ifndef S3VL_DATASET_CALLBACKS

#include <string>
#include <hdf5.h>
#include "s3vl_dataset_obj.h"
#include "s3vl_file_callbacks.h"
#include "operators.h"
using namespace std;

class S3VLDatasetCallbacks{
public:
	static void *S3VL_dataset_create(void *obj, const H5VL_loc_params_t *loc_params, const char *name, hid_t lcpl_id, hid_t type_id, hid_t space_id, hid_t dcpl_id, hid_t dapl_id, hid_t dxpl_id, void **req);
	static void *S3VL_dataset_open(void *obj, const H5VL_loc_params_t *loc_params, const char *name, hid_t dapl_id, hid_t dxpl_id, void **req);
	static herr_t S3VL_dataset_read(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, hid_t plist_id, void *buf, void **req);
	static herr_t S3VL_dataset_write(void *dset, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id, hid_t plist_id, const void *buf, void **req);
	static herr_t S3VL_dataset_get(void *dset, H5VL_dataset_get_args_t *args, hid_t dxpl_id, void **req);
	static herr_t S3VL_dataset_close(void *dset, hid_t dxpl_id, void **req);
};
#define S3VL_DATASET_CALLBACKS
#endif