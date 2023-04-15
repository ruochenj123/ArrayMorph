import sys
import json
import time
import os

workload = []
with open(sys.argv[2], 'r') as f:
    contents = f.readlines()
    for c in contents[2:]:
        workload.append(c.split()[0])
for q in workload:
    command = "python3 /home/jiang.2091/antrax/tiledb/src/tdb.py {0} {1}".format(sys.argv[1], q)
    os.system(command)
print("finish")
