import sys
import json
import time
import tiledb


workload = {}
with open(sys.argv[1], 'r') as f:
    workload = json.load(f)
keys = ['input_ids', 'input_mask', 'segment_ids', 'masked_lm_positions', 'masked_lm_ids', 'next_sentence_labels']


config = tiledb.Config()
config["vfs.s3.scheme"] = "http" 
config["vfs.s3.region"] = "us-east-1"
config["vfs.s3.endpoint_override"] = ""
config["vfs.s3.logging_level"] = "TRACE"
config["vfs.s3.use_virtual_addressing"] = "true"
tdb_ctx = tiledb.Ctx(config=config)

uri = "s3://bert-tdb/"


s = time.time()
for fname in workload: 
    for idx in workload[fname]:
        for key in keys:
            dset = tiledb.open(uri + fname + "/" + key)
            data = dset[idx]
e = time.time()
print("total time: ", e-s)
