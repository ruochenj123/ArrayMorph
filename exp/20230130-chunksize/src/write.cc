// #include "s3vl_vol_connector.h"
#include <hdf5.h>
#include <stdlib.h>
#include <iostream>
//#include "operators.h"
#include <math.h>
#include <aws/core/Aws.h>
using namespace std;
int main(int argc, char **argv)
{
    string vol_name = argv[1]; 
    hsize_t X = stoi(argv[2]), Y = stoi(argv[3]);
    // hsize_t X = 1024, Y = 1024;
    hid_t   file_id, dset_id, space_id, dcpl_id;
    hsize_t chunk_dims[2] = {X, Y};
    hsize_t dset_dims[2] = {X * 64, Y * 64};
    herr_t status;
    int *data = new int[10];
    memset(data, 0, 10);
    hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);

    // register vol
    // cout << "vol enabled" << endl;
    hid_t vol_id = H5VLregister_connector_by_name(vol_name.c_str(), H5P_DEFAULT);
    if (H5Pset_vol(fapl, vol_id, NULL) < 0)
        printf("vol failed\n");
    Aws::SDKOptions options;
    // options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Debug;

    //The AWS SDK for C++ must be initialized by calling Aws::InitAPI.
    Aws::InitAPI(options);
    string filename = "chunk_" + to_string(X) + "_" + to_string(Y) + ".h5";
    file_id =  H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, fapl);

    dcpl_id = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(dcpl_id, 2, chunk_dims);

    /* Create the dataspace and the chunked dataset */
    space_id = H5Screate_simple(2, dset_dims, NULL);
    dset_id = H5Dcreate(file_id, "test", H5T_NATIVE_INT, space_id, H5P_DEFAULT, dcpl_id, 
                        H5P_DEFAULT);
    status = H5Dwrite(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

    cout << "write successfully" << endl;
    H5Dclose(dset_id);
    H5Fclose(file_id);
    delete[] data;
    H5VLunregister_connector(vol_id);
    Aws::ShutdownAPI(options);
    return 0;

} /* end main() */

