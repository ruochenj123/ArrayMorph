import sys
import json
import time
import os
import h5pyd as h5py
workload = []
with open(sys.argv[1], 'r') as f:
    contents = f.readlines()
    for c in contents[2:]:
        workload.append(int(c.split(" ")[0]))

s = time.time()
with h5py.File("/home/admin/ant", "r") as f:
    dset = f["test"]
    for q in workload:
        data = dset[q]
e = time.time()
print("total time: ", e-s)