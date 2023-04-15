import os
import h5py
import random
import numpy as np
import json
rate = 0.1

input_dir="/home/jiang.2091/wikicorpus_en/training/test"
files = os.listdir(input_dir)

re = {}
for file in files:
    path = os.path.join(input_dir, file)
    f = h5py.File(path, "r")
    size = len(f['input_ids'])
    sample_size = int(size * rate)
    out = random.sample(range(size), sample_size)
    re[file] = out
    f.close()


with open("workload.json", "w") as f:
    json.dump(re, f)