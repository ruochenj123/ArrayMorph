#include "s3vl_file_callbacks.h"
#include <stdlib.h>
#include "logger.h"
#include "constants.h"

void* S3VLFileCallbacks::S3VL_file_create(const char *name, unsigned flags, hid_t fcpl_id, hid_t fapl_id, hid_t dxpl_id, void **req) {
	S3VLFileObj *ret_obj = new S3VLFileObj();
	ret_obj->name = name;
	Logger::log("------ Create File: ", name);
	return (void*)ret_obj;
}
void* S3VLFileCallbacks::S3VL_file_open(const char *name, unsigned flags, hid_t fapl_id, hid_t dxpl_id, void **req) {
	S3VLFileObj *ret_obj = new S3VLFileObj();
	ret_obj->name = name;
	Logger::log("------ Open File: ", name);
	return (void*)ret_obj;

}
herr_t S3VLFileCallbacks::S3VL_file_get(void *file, H5VL_file_get_args_t *args, hid_t dxpl_id, void **req) {
	return SUCCESS;
}
herr_t S3VLFileCallbacks::S3VL_file_close(void *file, hid_t dxpl_id, void **req) {
	S3VLFileObj *file_obj = (S3VLFileObj*)file;
	Logger::log("------ Close File: ", file_obj->name);
	delete file_obj;
	return SUCCESS;
}