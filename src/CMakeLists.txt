# Copyright (c) 2017 Matthias Noack <ma.noack.pr@gmail.com>
#
# See accompanying file LICENSE and README for further information.

# do not put executable into subdir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# example application
add_executable(ocl_info ocl_info.cpp)
target_link_libraries(ocl_info noma_ocl ${OpenCL_LIBRARIES})

