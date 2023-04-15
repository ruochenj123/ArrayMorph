import h5pyd as h5py
import sys
import json
import time

workload = {}
with open(sys.argv[1], 'r') as f:
    workload = json.load(f)
keys = ['input_ids', 'input_mask', 'segment_ids', 'masked_lm_positions', 'masked_lm_ids', 'next_sentence_labels']
s = time.time()

for fname in workload: 
    with h5py.File(fname, "r") as f:
    for idx in workload[fname]:
        for key in keys:
            dset = f[key]
            data = dset[idx]
e = time.time()
print("total time: ", e-s)
