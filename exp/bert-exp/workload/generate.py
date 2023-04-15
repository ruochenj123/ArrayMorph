import os
import h5py
import random
import numpy as np
import json
import argparse
import random

parser = argparse.ArgumentParser()
fname = "/home/jiang.2091/wikicorpus_en/training/wikicorpus_en_training_0.hdf5"
f = h5py.File(fname, "r")
size = len(f['input_ids'])
dsets = f.keys()

parser.add_argument('-s', dest='sel', type=float)
args = parser.parse_args();
num = 250
random.seed(1)
qs = []
for j in range(num):
    x = random.randint(0, size - int(size * args.sel))
    qs.append([x, x + int(size * args.sel) - 1])
for dset in dsets:
    path = dset + ".txt"
    with open(path, 'w') as of:
        of.write("{0}\n".format(f[dset].ndim))
        if dset != "next_sentence_labels":
            col_num = f[dset].shape[1]
            of.write("{0} {1}\n".format(size, col_num))
            for idx in qs:
                of.write("{0} {1} {2} {3}\n".format(idx[0], idx[1], 0, col_num - 1))
        else:
            of.write("{0}\n".format(size))
            for idx in qs:
                of.write("{0} {1}\n".format(idx[0], idx[1]))
