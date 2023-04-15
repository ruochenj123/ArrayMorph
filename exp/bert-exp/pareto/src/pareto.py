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

for k in keys:
    command = "/home/jiang.2091/bert-exp/pareto/build/build/read {0} {1}".format(k, sys.argv[1])
    os.system(command)
    
