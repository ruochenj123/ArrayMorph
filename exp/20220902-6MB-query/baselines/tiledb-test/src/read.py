import tiledb
import numpy as np
import time
import os
start = time.time()
config = tiledb.Config()
config["vfs.s3.scheme"] = "http" 
config["vfs.s3.region"] = "us-east-1"
config["vfs.s3.endpoint_override"] = ""
config["vfs.s3.logging_level"] = "TRACE"
config["vfs.s3.use_virtual_addressing"] = "true"
tdb_ctx = tiledb.Ctx(config=config)

uri = "s3://tiledb-hyperslab-test/tiledb"

a = tiledb.open(uri)
b = a[1:32768, 1:48]
#b = a[1021:15364, 1021:15364]
#b = a[8:32760, 8:32760]
#b = a[1:1021, 1:1024]
#print(b)
end = time.time()
print("total time:", end-start)
#os.system("/home/jiang.2091/clear_cache/build/clear_cache")
