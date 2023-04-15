import time
import h5pyd as h5py
start = time.time()
with h5py.File("/home/admin/hsds.hdf5", 'r') as f:
    dset = f['test']
    data = dset[0:32767,0:47]
#    data = dset[1021:15364, 1021:15364]
#    data = dset[8:32760, 8:32760]
#    data = dset[1:100, 1:100]
#    print(data)
    end = time.time()
    print("total time: ", end - start)

