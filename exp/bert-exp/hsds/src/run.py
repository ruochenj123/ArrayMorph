import sys
import json
import time
import os

workload = []
with open(sys.argv[1], 'r') as f:
    contents = f.readlines()
    for c in contents[2:]:
        workload.append([int(c.split(" ")[0]), int(c.split(" ")[1])])
keys = ['input_ids', 'input_mask', 'segment_ids', 'masked_lm_positions', 'masked_lm_ids', 'next_sentence_labels']

for q in workload:
    command = "python3 /home/jiang.2091/bert-exp/hsds/src/hsds.py {0} {1}".format(q[0], q[1])
    os.system(command)
print("finish")
