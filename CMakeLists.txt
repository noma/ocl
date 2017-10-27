# Copyright (c) 2017 Matthias Noack <ma.noack.pr@gmail.com>
#
# See accompanying file LICENSE and README for further information.

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(libnoma_ocl LANGUAGES CXX)

# include  kernel header generation CMake Module
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
include(CreateOpenCLKernelHeader)
#message("CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH})

set(NOMA_OCL_THIRDPARTY_DIR ${PROJECT_SOURCE_DIR}/thirdparty)

#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

# OpenCL
find_package(OpenCL REQUIRED)
set(OpenCL_INCLUDE_DIRS ${NOMA_OCL_THIRDPARTY_DIR}/include)

# Boost
find_package(Boost REQUIRED COMPONENTS program_options filesystem)
# avoid compiler warnings from Boost.System for unused symbols
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_SYSTEM_NO_DEPRECATED=1")

# static library
add_library(noma_ocl STATIC src/noma/ocl/config.cpp src/noma/ocl/device_type_list.cpp src/noma/ocl/helper src/noma/ocl/kernel_wrapper.cpp)

# NOTE: we want to use '#include "noma/ocl/ocl.hpp"', not '#include "ocl.hpp"'
target_include_directories(noma_ocl PUBLIC include ${OpenCL_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

target_link_libraries(noma_ocl noma_bmt noma_typa noma_memory ${OpenCL_LIBRARIES} ${Boost_LIBRARIES})

set_target_properties(noma_ocl PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

add_subdirectory(src)

option(NOMA_OCL_STANDALONE "Perform standalone build, i.e. pull in other dependencies.")

if(${NOMA_OCL_STANDALONE})
	add_subdirectory(thirdparty/bmt/ ${CMAKE_CURRENT_BINARY_DIR}/build.noma_bmt)
	add_subdirectory(thirdparty/typa/ ${CMAKE_CURRENT_BINARY_DIR}/build.noma_typa)
	add_subdirectory(thirdparty/misc/ ${CMAKE_CURRENT_BINARY_DIR}/build.noma_misc)
endif()
