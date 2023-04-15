#!/bin/bash

export HDF5_PLUGIN_PATH=/home/jiang.2091/s3-hyperslab/vol/tmp/src

# write data
../build/write 1
../build/write 4
../build/write 16
../build/write 64
../build/write 256
../build/write 1024
