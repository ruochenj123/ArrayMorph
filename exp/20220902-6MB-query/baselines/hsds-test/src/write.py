import h5pyd as h5py
import numpy as np

f = h5py.File('/home/admin/hsds.hdf5', 'w')
dset = f.create_dataset("test", (32768, 32768), chunks=(1024, 1024), dtype=np.int32)
data = np.ones((32768, 32768), dtype=np.int32)
dset[:] = data


