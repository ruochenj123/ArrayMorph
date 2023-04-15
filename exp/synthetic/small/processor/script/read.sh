#!/bin/bash

#VOL_PATH=/home/jiang.2091/s3-hyperslab/vol/tmp/src
#export HDF5_PLUGIN_PATH=${VOL_PATH}

# read data
rm -rf ../results/*
python randomization.py ../results/output.txt ../results/log.txt
