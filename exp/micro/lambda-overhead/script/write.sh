#!/bin/bash

#export HDF5_PLUGIN_PATH=/home/jiang.2091/s3-hyperslab/vol/tmp/src

# write data

p="../build/build/write"
declare -a vols=("s3_vol")
for v in "${vols[@]}"
do
    ${p} ${v} 1024 512
    ${p} ${v} 2048 512
    ${p} ${v} 4096 512
    ${p} ${v} 8192 512
    ${p} ${v} 16384 512
    echo "write {v} complete"
done
