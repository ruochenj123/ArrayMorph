#!/bin/bash

#export HDF5_PLUGIN_PATH=/home/jiang.2091/s3-hyperslab/vol/tmp/src

# write data

p="../build/build/write"
#declare -a vols=("azure_write" "s3_write" "gc_write")
#declare -a xs=("1024" "1448" "2048")
declare -a vols=("s3_write")
declare -a xs=("724")
for v in "${vols[@]}"
do
for x in "${xs[@]}"
do
    ${p} ${v} small_${x} ${x} ${x}
    echo "write ${v} complete"
done
done
