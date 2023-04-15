import os

input_file="/home/jiang.2091/wikicorpus_en/training/wikicorpus_en_training_0.hdf5"
keys = ['input_ids', 'input_mask', 'segment_ids', 'masked_lm_positions', 'masked_lm_ids', 'next_sentence_labels']
new_file = "wiki.hdf5"
for k in keys:
    os.system("../build/build/write {0} {1} {2}".format(input_file, k, new_file))
