import sys
import json
import time
import tiledb
import os
import argparse


config = tiledb.Config()
config["sm.tile_cache_size"] = 0
if sys.argv[1]  == "s3":
    config["vfs.s3.region"] = "us-east-1"
    config["vfs.s3.endpoint_override"] = ""
    config["vfs.s3.logging_level"] = "OFF"
    config["vfs.s3.use_virtual_addressing"] = "true"
    uri = "s3://bert-tdb/wiki"
elif sys.argv[1] == "azure":
    config["vfs.azure.storage_account_name"] = "ruochenj2"
    config["vfs.azure.storage_account_key"] = "LR6+fuNp/4PTt2Ie9ZJf5X1NBKg098qMv1pV5v2G/3tbbhyIITxcxAFjBaOTO/SLQ1Lk/l9SbNyp+AStGYLdEg=="
    uri = "azure://bert-tdb/wiki"
else:
    config["vfs.gcs.project_id"] = "river-hold-380321"
    uri = "gcs://bert-tdb/wiki"
tdb_ctx = tiledb.Ctx(config=config)

keys = ['input_ids', 'input_mask', 'segment_ids', 'masked_lm_positions', 'masked_lm_ids', 'next_sentence_labels']
s = time.time()
for k in keys:
    dset = tiledb.open(uri + "_" + k, "r", ctx=tdb_ctx)
    data = dset[int(sys.argv[2]):int(sys.argv[3]) + 1,]
#print(key)
#print(data)
e = time.time()
print("total time: ", e-s)
#os.system("/home/jiang.2091/clear_cache/build/clear_cache")
