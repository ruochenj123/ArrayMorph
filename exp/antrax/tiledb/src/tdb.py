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
    config["vfs.s3.logging_level"] = "Debug"
    config["vfs.s3.use_virtual_addressing"] = "true"
    uri = "s3://bert-tdb/ant_test/"
elif sys.argv[1] == "azure":
    config["vfs.azure.storage_account_name"] = "ruochenj323"
    config["vfs.azure.storage_account_key"] = "+FKO2ik6UaOZVdXzmWOdmcdifyxFzHcmc/wPa2cmpIYuPBDjJFmMXxy4mukGlk+TER99H6rXZPFk+AStCK7uHw=="
    uri = "azure://bert-tdb/ant_test/"
else:
    config["vfs.gcs.project_id"] = "river-hold-380321"
    uri = "gcs://bert-tdb/ant_test/"
tdb_ctx = tiledb.Ctx(config=config)

s = time.time()
dset = tiledb.open(uri, "r", ctx=tdb_ctx)
data = dset[int(sys.argv[2])]
#print(key)
#print(data)
e = time.time()
print("total time: ", e-s)
#os.system("/home/jiang.2091/clear_cache/build/clear_cache")
