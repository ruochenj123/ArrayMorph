#include <hdf5.h>
#include <stdlib.h>
#include <iostream>
#include <aws/core/Aws.h>
#include <sys/time.h>
using namespace std;


int main(int argc, char** argv)
{
    struct timeval start, end;
    gettimeofday(&start, NULL);

    hid_t   file_id, dset_id, space_id, dcpl_id;
    herr_t status;
    string vol_name = argv[5];
    int x = stoi(argv[1]);
    int y = stoi(argv[2]);
    int cx = stoi(argv[3]);
    int cy = stoi(argv[4]);
    int *out = new int[cx * cy];
    hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);
    // register vol
    hid_t vol_id = H5VLregister_connector_by_name(vol_name.c_str(), H5P_DEFAULT);
    if (H5Pset_vol(fapl, vol_id, NULL) < 0)
        printf("vol failed\n");
    Aws::SDKOptions options;
    // options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Debug;

    //The AWS SDK for C++ must be initialized by calling Aws::InitAPI.
    Aws::InitAPI(options);

    string filename = "mv_workload.h5"; 
    file_id = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, fapl);
    dset_id = H5Dopen(file_id, "test", H5P_DEFAULT);
    hsize_t offset[2] = {x, y};
    hsize_t offset_out[2] = {0, 0};
    hsize_t count[2] = {cx, cy};

    hid_t dataspace = H5Dget_space(dset_id);
    H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL, count, NULL);


    hid_t memspace = H5Screate_simple(2, count, NULL);

    status = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, offset_out, NULL, count, NULL);

    status = H5Dread(dset_id, H5T_NATIVE_INT, memspace, dataspace, H5P_DEFAULT, out);
    

    H5Sclose(memspace);
    H5Sclose(dataspace);

    H5Dclose(dset_id);
    H5Fclose(file_id);
    H5VLunregister_connector(vol_id);
    Aws::ShutdownAPI(options);


    gettimeofday(&end, NULL);
    double t = (1000000 * ( end.tv_sec - start.tv_sec )
		                + end.tv_usec -start.tv_usec) /1000000.0;
    cout << "Total Time: " << t << endl;
    system("/home/jiang.2091/clear_cache/build/clear_cache");
    return 0;

} /* end main() */
