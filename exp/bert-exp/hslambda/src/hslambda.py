import h5pyd as h5py
import sys
import json
import time

keys = ['input_ids', 'input_mask', 'segment_ids', 'masked_lm_positions', 'masked_lm_ids', 'next_sentence_labels']
with h5py.File("/home/admin/wiki", "r") as f:
    for key in keys:
        dset = f[key]
        data = dset[int(sys.argv[1]):int(sys.argv[2]),]
    
