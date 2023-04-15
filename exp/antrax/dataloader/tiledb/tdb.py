import tiledb
import numpy as np
import h5py
import os
import argparse
parser = argparse.ArgumentParser()
parser.add_argument('-p', dest="platform")
args = parser.parse_args()
config = tiledb.Config()
if args.platform == "s3":
    config["vfs.s3.scheme"] = "http" 
    config["vfs.s3.region"] = "us-east-1"
    config["vfs.s3.endpoint_override"] = ""
    config["vfs.s3.logging_level"] = "OFF"
    config["vfs.s3.use_virtual_addressing"] = "true"
    uri = "s3://bert-tdb/"
elif args.platform == "azure":
    #config["vfs.azure.blob_endpoint"] = "ruochenj2.blob.core.windows.net"
    config["vfs.azure.storage_account_name"] = "ruochenj323"
    config["vfs.azure.storage_account_key"] = "+FKO2ik6UaOZVdXzmWOdmcdifyxFzHcmc/wPa2cmpIYuPBDjJFmMXxy4mukGlk+TER99H6rXZPFk+AStCK7uHw=="
    uri = "azure://bert-tdb/"
else:
    config["vfs.gcs.project_id"] = "river-hold-380321"
    uri = "gcs://bert-tdb/"
tdb_ctx = tiledb.Ctx(config=config)

# Create the two dimensions

def create_dataset(f, fname, key):
    shape = f[key].shape
    print("shape: ", shape)
    chunk = f[key].chunks
    print("chunk: ", chunk)
    d1 = tiledb.Dim(name="d1", domain=(1, shape[0]), tile=chunk[0], dtype=np.int32)
    d2 = tiledb.Dim(name="d2", domain=(1, shape[1]),tile=chunk[1], dtype=np.int32)
    d3 = tiledb.Dim(name="d3", domain=(1, shape[2]),tile=chunk[2], dtype=np.int32)
    d4 = tiledb.Dim(name="d4", domain=(1, shape[3]),tile=chunk[3], dtype=np.int32)    
    # Create a domain using the two dimensions
    dom = tiledb.Domain(d1, d2, d3, d4)

    # Create an attribute
    a = tiledb.Attr(name="a", dtype=np.int8)

    # Create the array schema, setting `sparse=False` to indicate a dense array
    schema1 = tiledb.ArraySchema(domain=dom, sparse=False, attrs=[a])

    # Create the array on disk (it will initially be empty)
    outfile = uri + fname + "_" + key
    print("creating: ", outfile)
    tiledb.Array.create(outfile, schema1, ctx=tdb_ctx)

    with tiledb.open(outfile, mode='w', ctx=tdb_ctx) as A:
        data = np.asarray(f[key][:])
        A[:] = data

input_file = "/home/jiang.2091/antrax/sample.hdf5"
f = h5py.File(input_file, "r")

create_dataset(f, "ant", "test")

