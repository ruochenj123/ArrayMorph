#include <hdf5.h>
#include <stdlib.h>
#include <iostream>
#include <aws/core/Aws.h>
#include <sys/time.h>
#include <chrono>
#include <fstream>
#include <sstream>
using namespace std;


int main(int argc, char** argv)
{
    auto start_stamp = chrono::system_clock::now().time_since_epoch();
    auto start_millis = chrono::duration_cast<chrono::milliseconds>(start_stamp).count();
    cout << "start_stamp: " << start_millis << endl;

    hid_t   file_id, dset_id, space_id, dcpl_id;
    herr_t status;

    hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);
    // register vol
    string vol = "s3_pareto";
    hid_t vol_id = H5VLregister_connector_by_name(vol.c_str(), H5P_DEFAULT);
    if (H5Pset_vol(fapl, vol_id, NULL) < 0)
        printf("vol failed\n");
    Aws::SDKOptions options;
    //options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
    options.httpOptions.installSigPipeHandler = true;

    //The AWS SDK for C++ must be initialized by calling Aws::InitAPI.
    Aws::InitAPI(options);
    string file_name = "antrax";
    string dset_name = "test";
    ifstream input(argv[1]);
    vector<int> qs;
    string line;
    getline(input,line);
    getline(input,line);
    while(getline(input, line)) {
        int l;
        istringstream iss(line);
        iss >> l;
        qs.push_back(l);
    }
    struct timeval start, end;
    double t = 0.0;
    file_id = H5Fopen(file_name.c_str(), H5F_ACC_RDONLY, fapl);
        dset_id = H5Dopen(file_id, dset_name.c_str(), H5P_DEFAULT);
        hid_t dspace = H5Dget_space(dset_id);
        int ndims = H5Sget_simple_extent_ndims(dspace);

        hsize_t dims[ndims];
        H5Sget_simple_extent_dims(dspace, dims, NULL);
        cout << dset_name << endl;
        for (int i = 0; i < ndims; i++)
            cout << dims[i] << endl;
    for (auto &x: qs) {
        char *out = new char[1200 * 1200 * 3];
        hsize_t mem_out[4] = {1, 1200, 1200, 3};
        hsize_t count_out[4] = {1, 1200, 1200, 3};
        hsize_t offset_out[4] = {0, 0, 0, 0};

        // hid_t dataspace = H5Dget_space(dset_id);
        hid_t memspace = H5Screate_simple(4, mem_out, NULL);

        hsize_t offset[4] = {x, 0, 0, 0};
        hsize_t count[4] = {1, 1200, 1200, 3};
            
        H5Sselect_hyperslab(dspace, H5S_SELECT_SET, offset, NULL, count, NULL);
    
        status = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, offset_out, NULL, count_out, NULL);
        gettimeofday(&start, NULL);
        status = H5Dread(dset_id, H5T_NATIVE_CHAR, memspace, dspace, H5P_DEFAULT, out);
        gettimeofday(&end, NULL);
        t += (1000000 * ( end.tv_sec - start.tv_sec )
                        + end.tv_usec -start.tv_usec) /1000000.0;
        H5Sclose(memspace);
        delete[] out;
    }
        H5Dclose(dset_id);
        // delete[] out;
    H5Fclose(file_id);
    H5VLunregister_connector(vol_id);
    Aws::ShutdownAPI(options);
    auto end_stamp = chrono::system_clock::now().time_since_epoch();
    auto end_millis = chrono::duration_cast<chrono::milliseconds>(end_stamp).count();
    cout << "end_stamp: " << end_millis << endl;
    cout << "Total Time: " << t << endl;
    return 0;

} /* end main() */

