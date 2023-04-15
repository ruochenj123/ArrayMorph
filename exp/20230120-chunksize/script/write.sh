#!/bin/bash

#export HDF5_PLUGIN_PATH=/home/jiang.2091/s3-hyperslab/vol/tmp/src

# write data
../build/build/write azure_vol
echo "azure write complete"
../build/build/write gc_vol
echo "gc write complete"
../build/build/write s3_vol
echo "s3 write complete"
