# ArrayMorph

## Requirements
```
CMake>2.6.0
HDF5>=1.14.2
aws-sdk-cpp-s3=1.9.269
azure-storage-cpp=7.5.0
google-cloud-cpp=2.6.0
h5py>=3.10.0 (if using python)
```
AWS,Azure and Google SDK are recommended to be installed using vcpkg

## Install
```
$git clone https://github.com/ruochenj123/ArrayMorph.git
$cd $HOME/ArrayMorph/arraymorph
$cmake -B ./build -S . -DCMAKE_PREFIX=PATH_TO_HDF5 -DCMAKE_TOOLCHAIN_FILE=PATH_TO_VCPKG/vcpkg/scripts/buildsystems/vcpkg.cmake
$cd build
$make
```
## Environment variables
```
# For enable VOL
export HDF5_PLUGIN_PATH=$HOME/ArrayMorph/arraymorph/build/src
export HDF5_VOL_CONNECTOR=arraymorph

# For cloud bucket
export BUCKET_NAME=XXXXXX

# using S3
export STORAGE_PLATFORM=S3
export AWS_ACCESS_KEY_ID=XXXX
export AWS_SECRET_ACCESS_KEY=XXXX
export AWS_LAMBDA_ACCESS_POINT=XXXX

# using Azure
export STORAGE_PLATFORM=Azure
export AZURE_STORAGE_CONNECTION_STRING=XXXX
export AZURE_LAMBDA_URL=XXXX

# using GCS
export STORAGE_PLATFORM=Google
export GOOGLE_CLOUD_STORAGE_JSON=XXXX
export GC_LAMBDA_URL=XXXX
````
For S3, Azure, and GCS, only one of them is necessary to be set. Alternatively, you can set the following environment variable to enable single-operator processing method
```
export SINGLE_PLAN=XXX # GET, MERGE, or LAMBDA
```

## Usage
ArrayMorph supports both C++ and Python via h5py package. See examples in example folder.