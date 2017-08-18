# Copyright (c) 2017 Matthias Noack <ma.noack.pr@gmail.com>
#
# See accompanying file LICENSE and README for further information.

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(libnoma_ocl LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
include(CreateOpenCLKernelHeader)
message("CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH})

set(NOMA_OCL_THIRDPARTY_DIR ${PROJECT_SOURCE_DIR}/thirdparty)

#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

# TODO: add OpenCL stuff
# OpenCL
find_package(OpenCL REQUIRED)
set(OpenCL_INCLUDE_DIRS ${NOMA_OCL_THIRDPARTY_DIR}/include)

# TODO: write CMake module for code handling, move thirdparty module into this subproject

# header only library 
add_library(noma_ocl STATIC src/noma/ocl/config.cpp src/noma/ocl/device_type_list.cpp src/noma/ocl/helper src/noma/ocl/kernel_wrapper.cpp)

# NOTE: we want to use '#include "noma/ocl/ocl.hpp"', not '#include "ocl.hpp"'
target_include_directories(noma_ocl PUBLIC include ${OpenCL_INCLUDE_DIRS})

target_link_libraries(noma_ocl noma_bmt noma_typa ${OpenCL_LIBRARIES})

set_target_properties(noma_ocl PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

add_subdirectory(src)

