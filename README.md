
# ArrayMorph
ArrayMorph is a software to manage array data stored on cloud object storage efficiently. It supports both HDF5 C++ API and h5py API. The data returned by h5py API is numpy arrays. By using h5py API, user can access array data stored on cloud and feed the read data into machine learning pipelines seamlessly. For usage of ArrayMorph, please refer to examples in example folder.
  

## Requirements


	CMake>=3.0
	HDF5>=1.14.0
	h5py>=3.10.0 (if using python)
	aws-sdk-cpp-s3
	azure-storage-blobs-cpp
	google-cloud-cpp
	pkg-config


AWS,Azure and Google SDK are recommended to be installed using vcpkg

## Install

### HDF5
1.Download the HDF5 (https://www.hdfgroup.org/packages/hdf5-1142-source/)

2.Build form source

	$ cd /path/to/HDF5 
	$ ./configure --prefix=/path/to/hdf5/build --enable-cxx
	$ make
	$ make install

	
### h5py
1.Install using pip3

	$ HDF5_DIR=/path/to/hdf5/build pip3 install --no-binary=h5py h5py

	
### Other SDKs (via vcpkg)
1.Install vcpkg

	$ git clone https://github.com/microsoft/vcpkg.git
	$ cd vcpkg
	$ ./bootstrap-vcpkg.sh # This may ask you install some packages 

2.Install SDKs (If build fails, it might be caused by missing pkg-config. Take a look at the traceback logs.)

	$ ./vcpkg install aws-sdk-cpp[s3]
	$ ./vcpkg install google-cloud-cpp
	$ ./vcpkg install azure-storage-blobs-cpp

### ArrayMorph

	$ git clone https://ruochenj@bitbucket.org/ruochenj/arraymorph.git
	$ cd arraymorph/arraymorph
	$ cmake -B ./build -S . -DCMAKE_PREFIX_PATH=path/to/hdf5/build -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
	$ cd build
	$ make

### OpenSSL Compatibility Note

When using **ArrayMorph** with **h5py** in MPI applications, you may encounter errors caused by OpenSSL version mismatches between the system's OpenSSL and vcpkg's static SDKs.

**Solution:**  
Install and link SDKs dynamically (e.g., `-DVCPKG_TARGET_TRIPLET=x64-linux-dynamic`) to ensure compatibility and avoid conflicts.


## Configurations


### All environment variables

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
	export AZURE_LAMBDA_ENDPOINT=XXXX

	# using GCS
	export STORAGE_PLATFORM=Google
	export GOOGLE_CLOUD_STORAGE_JSON=XXXX
	export GC_LAMBDA_URL=XXXX
	export GC_LAMBDA_ENDPOINT=XXXX

	# choosing a plan
	export SINGLE_PLAN=XXX
