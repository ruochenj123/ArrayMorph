import os
import h5py
import h5pyd
import numpy as np

input_file="/home/jiang.2091/wikicorpus_en/training/wikicorpus_en_training_0.hdf5"

f = h5py.File(input_file, "r")
keys = ['input_ids', 'input_mask', 'segment_ids', 'masked_lm_positions', 'masked_lm_ids','next_sentence_labels']
new_f = h5pyd.File('/home/admin/wiki', 'w')
for key in keys:
    print("processing ", key)
    data = np.asarray(f[key][:])
    shape = data.shape
    print("shape: ", shape)
    dset = new_f.create_dataset(key, shape, dtype='i4', chunks=True)
    dset[:] = data
new_f.flush()
new_f.close()
f.close()

