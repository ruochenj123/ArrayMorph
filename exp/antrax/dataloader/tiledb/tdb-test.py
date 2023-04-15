import tiledb
import numpy as np
import h5py

config = tiledb.Config()
config["vfs.s3.scheme"] = "http" 
config["vfs.s3.region"] = "us-east-1"
config["vfs.s3.endpoint_override"] = ""
config["vfs.s3.logging_level"] = "INFO"
config["vfs.s3.use_virtual_addressing"] = "true"
tdb_ctx = tiledb.Ctx(config=config)

uri = "s3://bert-tdb/"

dset = tiledb.open(uri + "ant_test")
print(dset.tile)
