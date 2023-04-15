import tiledb
import numpy as np
import h5py
import os

config = tiledb.Config()
config["vfs.s3.scheme"] = "http" 
config["vfs.s3.region"] = "us-east-1"
config["vfs.s3.endpoint_override"] = ""
config["vfs.s3.logging_level"] = "OFF"
config["vfs.s3.use_virtual_addressing"] = "true"
tdb_ctx = tiledb.Ctx(config=config)

uri = "s3://bert-tdb/"
# Create the two dimensions

def transfer(arr, d):
    return np.array(([tuple(l) for l in arr]), dtype=d)

def create_dataset(f, fname):
    size = len(f['input_ids'])
    d1 = tiledb.Dim(name="d1", domain=(1, size), dtype=np.int32)
    dom = tiledb.Domain(d1)

    # Create an attribute
    a0 = tiledb.Attr(name="a0", dtype=np.dtype([("", np.int32)] * 128))
    a1 = tiledb.Attr(name="a1", dtype=np.dtype([("", np.int32)] * 128))
    a2 = tiledb.Attr(name="a2", dtype=np.dtype([("", np.int32)] * 20))
    a3 = tiledb.Attr(name="a3", dtype=np.dtype([("", np.int32)] * 20))
    a4 = tiledb.Attr(name="a4", dtype=np.dtype([("", np.int32)] * 128))
    a5 = tiledb.Attr(name="a5", dtype=np.int32)

    # Create the array schema, setting `sparse=False` to indicate a dense array
    schema1 = tiledb.ArraySchema(domain=dom, sparse=False, attrs=[a0, a1, a2, a3, a4, a5])
    # schema1 = tiledb.ArraySchema(domain=dom, sparse=False, attrs=[a0])
    # Create the array on disk (it will initially be empty)
    outfile = uri + fname
    print("creating: ", outfile)
    tiledb.Array.create(outfile, schema1)

    with tiledb.open(outfile, mode='w') as A:
        # A[:,:] = {"a0": np.asarray(f['input_ids'][:]),
        #         "a1": np.asarray(f['input_mask'][:]),
        #         "a2": np.asarray(f['masked_lm_ids'][:]),
        #         "a3": np.asarray(f['masked_lm_positions'][:]),
        #         "a4": np.asarray(f['segment_ids'][:]),
        #         "a5": np.asarray(f['next_sentence_labels'][:])}
        A[:] = {"a0": transfer(np.asarray(f['input_ids'][:]), np.dtype([("", np.int32)] * 128)),
                "a1": transfer(np.asarray(f['input_mask'][:]), np.dtype([("", np.int32)] * 128)),
                "a2": transfer(np.asarray(f['masked_lm_ids'][:]), np.dtype([("", np.int32)] * 20)),
                "a3": transfer(np.asarray(f['masked_lm_positions'][:]), np.dtype([("", np.int32)] * 20)),
                "a4": transfer(np.asarray(f['segment_ids'][:]), np.dtype([("", np.int32)] * 128)),
                "a5": np.asarray(f['next_sentence_labels'][:])}

input_dir="/home/jiang.2091/wikicorpus_en/training/example"
files = os.listdir(input_dir)

for file in files:
    path = os.path.join(input_dir, file)
    f = h5py.File(path, "r")
    create_dataset(f, file)

