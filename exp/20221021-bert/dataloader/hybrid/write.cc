#include <hdf5.h>
#include <stdlib.h>
#include <iostream>
#include <aws/core/Aws.h>
#include <sys/time.h>
#include <chrono>
using namespace std;


int main(int argc, char** argv)
{
    string file_name = argv[1];
    string dset_name = argv[2];

    hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);

    hid_t file = H5Fopen(file_name.c_str(), H5F_ACC_RDONLY, fapl);
    hid_t dset = H5Dopen(file, dset_name.c_str(), H5P_DEFAULT);

    hid_t dspace = H5Dget_space(dset);
    const int ndims = H5Sget_simple_extent_ndims(dspace);

    hsize_t dims[ndims];
    H5Sget_simple_extent_dims(dspace, dims, NULL);

    hsize_t total_size = 1;
    // cout << "rank: " << ndims << endl;
    for (int i = 0; i < ndims; i++) {
        // cout << i << " : " << dims[i] << " ";
        total_size *= dims[i];
    }

    hsize_t chunk_dims[ndims];
    for (int i = 0; i < ndims; i++)
    {
        chunk_dims[i] = dims[i] / 4;
    }

    int *data = new int[total_size];

    hid_t status = H5Dread(dset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);


    // hid_t   file_id, dset_id, space_id, dcpl_id;
    // herr_t status;

    
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
    

    file =  H5Fcreate(file_name.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, fapl);
    hid_t dcpl_id = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(dcpl_id, ndims, chunk_dims);

    /* Create the dataspace and the chunked dataset */
    hid_t space_id = H5Screate_simple(ndims, dims, NULL);
    dset = H5Dcreate(file, dset_name.c_str(), H5T_NATIVE_INT, space_id, H5P_DEFAULT, dcpl_id, 
                        H5P_DEFAULT);
    status = H5Dwrite(dset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

    cout << "write successfully" << endl;
    H5Dclose(dset);
    H5Fclose(file);
    delete[] data;
    H5VLunregister_connector(vol_id);
    Aws::ShutdownAPI(options);
    return 0;
} /* end main() */

