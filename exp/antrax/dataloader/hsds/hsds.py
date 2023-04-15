import os
import h5py
import h5pyd
import numpy as np

input_file = "/home/jiang.2091/sample.hdf5"
f = h5py.File(input_file, "r")

new_f = h5pyd.File('/home/admin/ant', 'w')
data = np.asarray(f["test"][:])
shape = data.shape
print("shape: ", shape)
dset = new_f.create_dataset("test", (32400,1200,1200,3), dtype='i1', chunks=True)
dset = new_f["test"]
#num = shape[0]
dset[:] = data[:]
#dset[0:num//2] = data[0:num//2]
#dset[num//2:num] = data[num//2:num]
new_f.flush()
new_f.close()
f.close()

