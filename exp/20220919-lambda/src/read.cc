#include <hdf5.h>
#include <stdlib.h>
#include <iostream>
#include <aws/core/Aws.h>
#include <sys/time.h>
#include <chrono>
using namespace std;


int main(int argc, char** argv)
{
    auto start_stamp = chrono::system_clock::now().time_since_epoch();
    auto start_millis = chrono::duration_cast<chrono::milliseconds>(start_stamp).count();
    cout << "start_stamp: " << start_millis << endl;
    struct timeval start, end;

    hid_t   file_id, dset_id, space_id, dcpl_id;
    herr_t status;

    hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);
    // register vol
    string returned_size = argv[1];
    string vol = "s3_vol_lambda_" + returned_size;
    hid_t vol_id = H5VLregister_connector_by_name(vol.c_str(), H5P_DEFAULT);
    if (H5Pset_vol(fapl, vol_id, NULL) < 0)
        printf("vol failed\n");
    Aws::SDKOptions options;
    //options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
    options.httpOptions.installSigPipeHandler = true;

    //The AWS SDK for C++ must be initialized by calling Aws::InitAPI.
    Aws::InitAPI(options);
    int op = stoi(argv[2]);
    string filename = "lambda_test/33554432.h5";
    file_id = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, fapl);
    dset_id = H5Dopen(file_id, "test", H5P_DEFAULT);
    
    int x = 0;
    uint64_t y1 = 2048 * 16;
    uint64_t y2 = 4096 * 16;
    int* out = new int[y1 * y2];
    hsize_t offset[2] = {x, x};
    hsize_t count[2] = {y1, y2};

    hsize_t offset_out[2] = {0, 0};
    hsize_t count_out[2];

    hid_t memspace = H5Screate_simple(2, count, NULL);


    hid_t dataspace = H5Dget_space(dset_id);
    H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL, count_out, NULL);

    status = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, offset_out, NULL, count, NULL);
    gettimeofday(&start, NULL);
    status = H5Dread(dset_id, H5T_NATIVE_INT, memspace, dataspace, H5P_DEFAULT, out);
    gettimeofday(&end, NULL);
    H5Sclose(memspace);

    H5Dclose(dset_id);
    H5Fclose(file_id);
    H5VLunregister_connector(vol_id);
    Aws::ShutdownAPI(options);


    double t = (1000000 * ( end.tv_sec - start.tv_sec )
		                + end.tv_usec -start.tv_usec) /1000000.0;
    cout << "Total Time: " << t << endl;
    system("/home/jiang.2091/clear_cache/build/clear_cache");
    auto end_stamp = chrono::system_clock::now().time_since_epoch();
    auto end_millis = chrono::duration_cast<chrono::milliseconds>(end_stamp).count();
    cout << "end_stamp: " << end_millis << endl;
    return 0;

} /* end main() */

