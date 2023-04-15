import sys
import json
import time
import tiledb
import os

workload = []
with open(sys.argv[1], 'r') as f:
    contents = f.readlines()
    for c in contents[2:3]:
        workload.append([int(c.split(" ")[0]), int(c.split(" ")[1])])
#print(workload)
keys = ['input_ids', 'input_mask', 'segment_ids', 'masked_lm_positions', 'masked_lm_ids', 'next_sentence_labels']


config = tiledb.Config()
config["vfs.s3.scheme"] = "http" 
config["vfs.s3.region"] = "us-east-1"
config["vfs.s3.endpoint_override"] = ""
config["vfs.s3.logging_level"] = "DEBUG"
config["vfs.s3.use_virtual_addressing"] = "true"
tdb_ctx = tiledb.Ctx(config=config)

uri = "s3://bert-tdb/wiki"


s = time.time()
for i,idx in enumerate(workload):
    for key in keys:
        dset = tiledb.open(uri + "_" + key)
        data = dset[idx[0]:idx[1],]
#print(key)
#print(data)
e = time.time()
print("total time: ", e-s)
#os.system("/home/jiang.2091/clear_cache/build/clear_cache")
