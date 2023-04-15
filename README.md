# ArrayMorph

## To compile
1. Use vcpkg to install the corresponding version (mentioned in paper) of aws-sdk-cpp-s3, azure-storage-cpp, and google-cloud-cpp.
2. Download and compile HDF5 1.13 (https://github.com/HDFGroup/hdf5/releases/tag/hdf5-1_13_0).
3. Download and compile ArrayMorph
```
git clone https://github.com/ruochenj123/azure-vol-test.git
cd $HOME/ArrayMorph/arraymorph
cmake -B ./build -S . -DCMAKE_TOOLCHAIN_FILE=PATH_TO_VCPKG/vcpkg/scripts/buildsystems/vcpkg.cmake
cd build
make
```
4. To use ArrayMorph, add environment variables including credentials of cloud storage accounts.
```
export AZURE_STORAGE_CONNECTION_STRING=XXXX
export BUCKET_NAME=XXXX
export HDF5_PLUGIN_PATH=$HOME/ArrayMorph/arraymorph/build/src
export AWS_ACCESS_KEY_ID=XXXX
export AWS_SECRET_ACCESS_KEY=XXXX
export GOOGLE_CLOUD_STORAGE_JSON=XXXX
export GC_LAMBDA_URL=XXXX
export GC_LAMBDA_TOKEN=XXXX
export AWS_LAMBDA_ACCESS_POINT=XXXX
````