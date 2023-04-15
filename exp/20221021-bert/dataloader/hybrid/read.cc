#include <hdf5.h>
#include <stdlib.h>
#include <iostream>
#include <aws/core/Aws.h>
#include <sys/time.h>
#include <chrono>
using namespace std;


int main(int argc, char** argv)
{

    hid_t   file_id, dset_id, space_id, dcpl_id;
    herr_t status;

    hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);
    // register vol
    string vol = "s3_vol_get";
    hid_t vol_id = H5VLregister_connector_by_name(vol.c_str(), H5P_DEFAULT);
    if (H5Pset_vol(fapl, vol_id, NULL) < 0)
        printf("vol failed\n");
    Aws::SDKOptions options;
    //options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
    options.httpOptions.installSigPipeHandler = true;

    //The AWS SDK for C++ must be initialized by calling Aws::InitAPI.
    Aws::InitAPI(options);
    string file_name = argv[1];
    int idx = stoi(argv[2]);
    string dset_name = argv[3];
    file_id = H5Fopen(file_name.c_str(), H5F_ACC_RDONLY, fapl);
    dset_id = H5Dopen(file_id, dset_name.c_str(), H5P_DEFAULT);
    
    hid_t dspace = H5Dget_space(dset_id);
    const int ndims = H5Sget_simple_extent_ndims(dspace);

    hsize_t dims[ndims];
    H5Sget_simple_extent_dims(dspace, dims, NULL);
    
    hsize_t out[256];
    hsize_t mem_out[2] = {1, 256};
    hsize_t count_out[2] = {1, dims[0]};
    hsize_t offset_out[2] = {0, 0};

    hid_t dataspace = H5Dget_space(dset_id);
    hid_t memspace = H5Screate_simple(2, mem_out, NULL);

    if (dset_name != "next_sentence_labels") {
        hsize_t offset[2] = {idx, 0};
        hsize_t count[2] = {1, dims[0]};
        
        H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL, count, NULL);
    }
    else {
        hsize_t offset[1] = {idx};
        hsize_t count[1] = {1};
        H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL, count, NULL);
    }

    
    status = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, offset_out, NULL, count_out, NULL);
    status = H5Dread(dset_id, H5T_NATIVE_INT, memspace, dataspace, H5P_DEFAULT, out);
    H5Sclose(memspace);

    H5Dclose(dset_id);
    H5Fclose(file_id);
    H5VLunregister_connector(vol_id);
    Aws::ShutdownAPI(options);

    return 0;

} /* end main() */

