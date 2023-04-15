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

keys = ['input_ids', 'input_mask', 'segment_ids', 'masked_lm_positions', 'masked_lm_ids','next_sentence_labels']
rand = 100
#for key in keys:
#    print(key)
#    #data = tiledb.open(uri + key)
#    #print(key, data)
#    s = tiledb.ArraySchema.load(uri + key)
#    print(s)
dset = tiledb.open(uri + "test")
data = dset[1:16,1:16]
print(data)
