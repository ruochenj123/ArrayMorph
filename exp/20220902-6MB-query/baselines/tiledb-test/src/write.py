import tiledb
import numpy as np

config = tiledb.Config()
config["vfs.s3.scheme"] = "http" 
config["vfs.s3.region"] = "us-east-1"
config["vfs.s3.endpoint_override"] = ""
config["vfs.s3.logging_level"] = "TRACE"
config["vfs.s3.use_virtual_addressing"] = "true"
tdb_ctx = tiledb.Ctx(config=config)

uri = "s3://tiledb-hyperslab-test/tiledb"
# Create the two dimensions
d1 = tiledb.Dim(name="d1", domain=(1, 32768), tile=1024, dtype=np.int32)
d2 = tiledb.Dim(name="d2", domain=(1, 32768), tile=1024, dtype=np.int32)

# Create a domain using the two dimensions
dom1 = tiledb.Domain(d1, d2)

# Create an attribute
a = tiledb.Attr(name="a", dtype=np.int32)

# Create the array schema, setting `sparse=False` to indicate a dense array
schema1 = tiledb.ArraySchema(domain=dom1, sparse=False, attrs=[a])

# Create the array on disk (it will initially be empty)
tiledb.Array.create(uri, schema1)

with tiledb.open(uri, mode='w') as A:
    data = np.ones((32768, 32768), dtype=np.int32)
    A[:] = data

