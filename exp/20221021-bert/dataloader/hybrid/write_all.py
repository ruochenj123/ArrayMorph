import os

input_dir="/home/jiang.2091/wikicorpus_en/training/test"
files = os.listdir(input_dir)
keys = ['input_ids', 'input_mask', 'segment_ids', 'masked_lm_positions', 'masked_lm_ids','next_sentence_labels']

for file in files:
    path = os.path.join(input_dir, file)
    for key in keys:
    	command = "/home/jiang.2091/dataloader/hybrid/write {0} {1}".format(file, key)
    	os.system(command)