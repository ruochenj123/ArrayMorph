import os
import h5py
import h5pyd
import numpy as np

input_dir="/home/jiang.2091/wikicorpus_en/training/test"
files = os.listdir(input_dir)

for file in files:
    path = os.path.join(input_dir, file)
    f = h5py.File(path, "r")
    keys = ['input_ids', 'input_mask', 'segment_ids', 'masked_lm_positions', 'masked_lm_ids','next_sentence_labels']
    print(file, path)
    new_f = h5pyd.File('/home/admin/' + file, 'w')
    for key in keys:
        print("processing ", key)
        data = np.asarray(f[key][:])
        shape = data.shape
        dset = new_f.create_dataset(key, shape, dtype='i4')
        dset[:] = data
    new_f.flush()
    new_f.close()
    f.close()

