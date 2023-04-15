#!/bin/bash

#export HDF5_PLUGIN_PATH=/home/jiang.2091/s3-hyperslab/vol/tmp/src

VOL="azure_vol"
# write data
#../build/build/write 1048576 ${VOL}
#../build/build/write 262144 ${VOL}
#../build/build/write 65536 ${VOL}
#../build/build/write 16384 ${VOL}
#../build/build/write 4096 ${VOL}
#../build/build/write 1024 ${VOL}
../build/build/write 256 ${VOL}
../build/build/write 64 ${VOL}
../build/build/write 16 ${VOL}
