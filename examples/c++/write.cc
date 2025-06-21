#include <hdf5.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

#define X 100
#define Y 100
int main(void)
{

    hid_t   file_id, dset_id, space_id, dcpl_id;
    hsize_t chunk_dims[2] = {10, 10};
    hsize_t dset_dims[2] = {X, Y};
    herr_t status;
    int data[X][Y];
    // write 100 x 100 2d arrays filled with value 1
    for (int i = 0; i < X; i++)
        for (int j = 0; j < Y; j++)
            data[i][j] = 1;

    hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);

    file_id =  H5Fcreate("test.h5", H5F_ACC_TRUNC, H5P_DEFAULT, fapl);

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

    return 0;

} /* end main() */
