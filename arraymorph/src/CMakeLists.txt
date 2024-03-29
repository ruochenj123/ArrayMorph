# Set the template VOL connector name
set (TVC_NAME arraymorph)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 14)
#Load required services/packages: This basic example uses S3.
find_package(AWSSDK REQUIRED COMPONENTS s3)
find_package(CURL REQUIRED)
find_package(azure-storage-blobs-cpp CONFIG REQUIRED)
find_package(google_cloud_cpp_storage CONFIG REQUIRED)
# find_package(OpenMP REQUIRED)
find_package(OpenSSL REQUIRED)

# Build the template VOL
add_library (${TVC_NAME} SHARED s3vl_vol_connector.cc)
add_library (CONSTANTS constants.cc)
add_library (OPERATORS operators.cc)
add_library (VL_CHUNK_OBJ s3vl_chunk_obj.cc)
add_library (QUERY_PROCESSOR query_processor.cc)
add_library (VL_FILE s3vl_file_callbacks.cc)
add_library (VL_DSET s3vl_dataset_callbacks.cc)
add_library (VL_DSET_OBJ s3vl_dataset_obj.cc)
add_library (UTILS utils.cc)
add_library (PROFILER profiler.cc)
add_executable (storage_planner storage_planner.cc)

target_link_libraries(OPERATORS PRIVATE Azure::azure-storage-blobs)
target_link_libraries(OPERATORS PRIVATE CURL::libcurl google-cloud-cpp::storage google-cloud-cpp::experimental-storage-grpc)
target_link_libraries(OPERATORS ${OPENSSL_LIBRARIES})
target_link_libraries(OPERATORS ${AWSSDK_LINK_LIBRARIES})
target_link_libraries(OPERATORS CONSTANTS)
target_link_libraries(OPERATORS PROFILER)
target_link_libraries(VL_CHUNK_OBJ OPERATORS)
target_link_libraries(VL_CHUNK_OBJ UTILS)
target_link_libraries(QUERY_PROCESSOR VL_CHUNK_OBJ)
target_link_libraries(VL_DSET_OBJ QUERY_PROCESSOR)
target_link_libraries(VL_DSET_OBJ VL_CHUNK_OBJ)
target_link_libraries(storage_planner VL_DSET_OBJ)
target_link_libraries (storage_planner ${HDF5_C_LIBRARIES})
# target_link_libraries(VL_DSET_OBJ OpenMP::OpenMP_CXX)
target_link_libraries(VL_DSET VL_FILE)
target_link_libraries(VL_DSET VL_DSET_OBJ)
target_link_libraries(${TVC_NAME} VL_DSET)
target_link_libraries(${TVC_NAME} VL_FILE)
target_link_libraries(${TVC_NAME} ${HDF5_C_LIBRARIES})

# set_target_properties (${TVC_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties (${TVC_NAME} PROPERTIES SOVERSION 1)
set_target_properties (${TVC_NAME} PROPERTIES PUBLIC_HEADER "${TVC_NAME}.h")
