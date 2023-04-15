#include <hdf5.h>
#include <stdlib.h>
#include <iostream>
#include <aws/core/Aws.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <sys/time.h>
using namespace std;


int main(int argc, char** argv)
{
    struct timeval start, end;
    double t = 0.0;
    
    auto start_stamp = chrono::system_clock::now().time_since_epoch();
    auto start_millis = chrono::duration_cast<chrono::milliseconds>(start_stamp).count();
    cout << "start_stamp: " << start_millis << endl;

    hid_t   file_id, dset_id, space_id, dcpl_id;
    herr_t status;
    string vol_name = argv[1];
    string workload = argv[2];
    string filename = argv[3];
    hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);
    // register vol
    hid_t vol_id = H5VLregister_connector_by_name(vol_name.c_str(), H5P_DEFAULT);
    if (H5Pset_vol(fapl, vol_id, NULL) < 0)
        printf("vol failed\n");
    Aws::SDKOptions options;
    Aws::InitAPI(options);

    file_id = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, fapl);
    dset_id = H5Dopen(file_id, "test", H5P_DEFAULT);

// read workload
    ifstream input(workload);
    string line;
    int ndims;

    input >> ndims;
    vector<hsize_t> dset_shape;
    getline(input, line);

    for (int i = 0; i < ndims; i++) {
        int tmp;
        input >> tmp;
        dset_shape.push_back(tmp);
    }
    getline(input, line);
    vector<vector<vector<hsize_t>>> queries;
    while(getline(input, line)) {
        istringstream iss(line);
        vector<vector<hsize_t>> q;
        for (int i = 0; i < ndims; i++) {
            size_t l, r;
            iss >> l;
            iss >> r;
            q.push_back({l, r});
        }
        queries.push_back(q);
    }
    cout << "qnum: " << queries.size() << endl;

    for (auto &q : queries) {
        hsize_t offset[2] = {q[0][0], q[1][0]};
        hsize_t offset_out[2] = {0, 0};
        hsize_t count[2] = {q[0][1] - q[0][0] + 1, q[1][1] - q[1][0] + 1};
        int *out = new int[count[0] * count[1]];

        hid_t dataspace = H5Dget_space(dset_id);
        H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL, count, NULL);


        hid_t memspace = H5Screate_simple(2, count, NULL);

        status = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, offset_out, NULL, count, NULL);

        gettimeofday(&start, NULL);
        status = H5Dread(dset_id, H5T_NATIVE_INT, memspace, dataspace, H5P_DEFAULT, out);
        gettimeofday(&end, NULL);
        t += (1000000 * ( end.tv_sec - start.tv_sec )
                        + end.tv_usec -start.tv_usec) /1000000.0;

        H5Sclose(memspace);
        H5Sclose(dataspace);
        delete[] out;
        // system("/home/jiang.2091/clear_cache/build/clear_cache");
    }

    H5Dclose(dset_id);
    H5Fclose(file_id);
    H5VLunregister_connector(vol_id);
    Aws::ShutdownAPI(options);
    auto end_stamp = chrono::system_clock::now().time_since_epoch();
    auto end_millis = chrono::duration_cast<chrono::milliseconds>(end_stamp).count();
    cout << "end_stamp: " << end_millis << endl;

    
    cout << "Total Time: " << t << endl;
    
    return 0;
    
} /* end main() */

