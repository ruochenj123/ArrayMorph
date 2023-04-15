cmake_minimum_required (VERSION 2.6)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-O0")



# Find HDF5
find_package (HDF5 1.13 COMPONENTS C REQUIRED)
link_directories (${HDF5_LIBRARY_DIRS})
include_directories (${HDF5_INCLUDE_DIR})
set (LINK_LIBS ${LINK_LIBS} ${HDF5_C_${LIB_TYPE}_LIBRARY})

# It's really easy to pick up the wrong HDF5 library if you set the path
# wrong. Turn this on for added confirmation that you got it right.
message (DEPRECATION "Include: ${HDF5_INCLUDE_DIR}")

#set(VOL_PATH "/home/jiang.2091/s3-hyperslab/vol")
#include_directories("${VOL_PATH}/src")
#link_directories("${VOL_PATH}/tmp/src/")

find_package(AWSSDK REQUIRED COMPONENTS s3)

add_executable (write ../src/write.cc)
target_link_libraries (write ${HDF5_C_LIBRARIES})
target_link_libraries (write ${AWSSDK_LINK_LIBRARIES})

add_executable (read ../src/read.cc)
target_link_libraries (read ${HDF5_C_LIBRARIES})
target_link_libraries (read ${AWSSDK_LINK_LIBRARIES})
