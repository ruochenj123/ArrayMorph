#!/bin/bash

#export HDF5_PLUGIN_PATH=/home/jiang.2091/s3-hyperslab/vol/tmp/src

VOL="gc_vol"
# write data
../build/build/write 1048576 ${VOL}
../build/build/write 262144 ${VOL}
../build/build/write 65536 ${VOL}
../build/build/write 16384 ${VOL}
../build/build/write 4096 ${VOL}
../build/build/write 1024 ${VOL}
#../build/write 256
#../build/write 64
#../build/write 16
