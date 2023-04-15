import os
import h5py
import random
import numpy as np
import json
import argparse
import random

parser = argparse.ArgumentParser()
frame_num = 3240 
parser.add_argument('-s', dest='sel', type=float)
args = parser.parse_args();
random.seed(1)
qs = []
num = int(args.sel * frame_num)
interval = int(1 / args.sel)
start = 1
for i in range(num):
    qs.append(start)
    start += interval
path = "workload.txt"
with open(path, 'w') as of:
    of.write("4\n")
    of.write("3240 1200 1200 3\n")
    for q in qs:
        of.write("{0} {1} 0 1199 0 1199 0 2\n".format(q, q + 1))
