import os
os.environ["SINGLE_PLAN"]="GET"
import h5py
import numpy as np
import math
import time
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--upload", action="store_true")
args = parser.parse_args()


def upload_chunks(chunk_sizes, uri):
    print(f"uploading one chunk of data", flush=True)
    fname = uri + "/microbenchmark_single_chunk.h5"
    f = h5py.File(fname, "w")
    for c in chunk_sizes:
        shape = (math.ceil(math.sqrt(c * 1024 * 1024 / 4)), math.ceil(math.sqrt(c * 1024 * 1024 / 4)))
        data = np.ones(shape, dtype=np.int32)
        dset = f.create_dataset(str(c), shape, dtype=np.int32)
        print(f"uploading dataset with chunk size {c}", flush=True)
        dset[:] = data

def upload(data, uri, cs):
    print(f"uploading benchmarking data", flush=True)
    fname = uri + "/microbenchmark.h5"
    f = h5py.File(fname, "w")
    for r in cs:
        dset = f.create_dataset(str(cs[r]), (65536, 65536), chunks=(cs[r],cs[r]), dtype=np.int32)
        print(f"uploading dataset with shape {cs[r]}", flush=True)
        dset[:] = data

def profile(uri, cs, nr):
    fname = uri + "/microbenchmark.h5"
    f = h5py.File(fname, "r")
    dset = f[str(cs[nr])]
    s = time.time()
    data = dset[:]
    e = time.time()
    print(f"Number of Requests: {nr}", flush=True)
    print(f"Read Time: {e - s}", flush=True)
    print(f"Average Throughput: {16 * 1024 / (e - s)} MB/s", flush=True)


if __name__ == "__main__":
    n_requests =[2 * 4**i for i in range(1, 10)]
    chunk_sizes = [2, 4, 8, 16, 32, 64]
    # n_requests = [32 * 1024, 128 * 1024, 512 * 1024]
    # n_requests = [512*1024]
    cs = {}
    for r in n_requests:
        cs[r] = math.ceil(65536 / math.sqrt(r))
    if args.upload:
        upload_start = time.time()
        data = np.random.randint(1, 100, size=(65536, 65536), dtype=np.int32)
        upload(data, "micro", cs)
        upload_chunks(chunk_sizes, "micro")
        upload_end = time.time()
        print(f"Write Datasets Successfully! Using {upload_end - upload_start} seconds.", flush=True)
    profile_start = time.time()
    for nr in n_requests:
        profile("micro", cs, nr)
    profile_end = time.time()
    print(f"Profiling finished. Using {profile_end - profile_start} seconds.", flush=True)
