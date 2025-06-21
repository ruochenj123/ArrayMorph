#include <hdf5.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

#define X 10
#define Y 10


#define OUT_X 20
#define OUT_Y 20

#define COUNT_X 10
#define COUNT_Y 10

int main(void)
{

    hid_t   file_id, dset_id, space_id, dcpl_id;
    herr_t status;

    hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);

    
    file_id = H5Fopen("test.h5", H5F_ACC_RDONLY, fapl);
    dset_id = H5Dopen(file_id, "test", H5P_DEFAULT);
    
    int out[X][X];
    // initialize output buffer
    for (int i = 0; i < X; i++)
        for (int j = 0; j < Y; j++)
            out[i][j] = -1;

    // read dset[5:10, 5:10]
    hsize_t offset[2], offset_out[2];
    hsize_t count[2] = {COUNT_X, COUNT_Y};
    hsize_t dims_out[2] = {COUNT_X, COUNT_Y};

    offset[0] = 5;
    offset[1] = 5;
    offset_out[0] = 0;
    offset_out[1] = 0;

    hid_t dataspace = H5Dget_space(dset_id);
    H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL, count, NULL);


    hid_t memspace = H5Screate_simple(2, dims_out, NULL);

    status = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, offset_out, NULL, count, NULL);

    status = H5Dread(dset_id, H5T_NATIVE_INT, memspace, dataspace, H5P_DEFAULT, out);
    for (int i = 0; i < X; i++) {
        for (int j = 0; j < Y; j++)
            cout << out[i][j] << " ";
        cout << endl;
    }

    H5Sclose(memspace);
    H5Sclose(dataspace);

    H5Dclose(dset_id);
    H5Fclose(file_id);
    return 0;

} /* end main() */