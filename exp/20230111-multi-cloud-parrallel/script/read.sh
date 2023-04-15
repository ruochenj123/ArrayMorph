#!/bin/bash

#VOL_PATH=/home/jiang.2091/s3-hyperslab/vol/tmp/src
#export HDF5_PLUGIN_PATH=${VOL_PATH}

# read data
python randomization.py ../results/output.txt ../results/log.txt
