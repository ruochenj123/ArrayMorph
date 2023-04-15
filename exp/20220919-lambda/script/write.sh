#!/bin/bash

export HDF5_PLUGIN_PATH=/home/jiang.2091/s3-hyperslab/vol/tmp/src

# write data
x=256
y=512

flag=1
for i in {1..7}
do
../build/write ${x} ${y}
    if (( flag == 1))
    then
        x=$((x*2))
        flag=2
    else
        y=$((y*2))
        flag=1
    fi
done
