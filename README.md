# OpenCL Helper Classes

A simple library for easier use of OpenCL in C++ projects, optionally with CMake support. 

Features include:
- easier building
- memory usage tracing
- OpenCL source code handling via CMake
- kernel-wrapper base class
	- source code handling
	- runtime benchmarking
- config-file base class
	- platform/device selection
	- compiler options
	- etc.

## Include it into a CMake-Project

Download:
```bash
git clone git@github.com:noma/ocl.git
```

Export the repository content into your project's `thirdparty` directory:
```bash
cd ocl
git checkout-index -a -f --prefix=../my_project/thirdparty/ocl/
cd ..
```

Do the same for the dependencies:
```
git clone git@github.com:noma/bmt.git
git clone git@github.com:noma/misc.git
git clone git@github.com:noma/typa.git

#...
```

In `myproject/CMakeLists.txt` add:
```cmake
add_subdirectory(../thirdparty/misc/ ${CMAKE_CURRENT_BINARY_DIR}/build.noma_misc)
# benchmark timer
add_subdirectory(../thirdparty/bmt/ ${CMAKE_CURRENT_BINARY_DIR}/build.noma_bmt)
# type parser library
add_subdirectory(../thirdparty/typa/ ${CMAKE_CURRENT_BINARY_DIR}/build.noma_typa)
# OpenCL helper library
add_subdirectory(../thirdparty/ocl/ ${CMAKE_CURRENT_BINARY_DIR}/build.noma_ocl)
```

Use `noma_ocl` as library dependency for your targets:
```cmake
add_executable(my_target my_target.cpp)
target_link_libraries(my_target noma_ocl)
# make sure at least C++11 is used
set_target_properties(my_target PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
```

## Standalone/Library Build

Clone with all dependencies:
```bash
git clone git@github.com:noma/ocl.git
git clone git@github.com:noma/bmt.git
git clone git@github.com:noma/misc.git
git clone git@github.com:noma/typa.git
```

Link depdendencies into `ocl/thirdparty`
```bash
cd ocl/thirdparty
ln -s ../../bmt/ bmt
ln -s ../../misc/ misc
ln -s ../../typa/ typa
cd ../
```

CMake build using the `NOMA_OCL_STANDALONE` option:
```bash
mkdir build
cd build
cmake -DNOMA_OCL_STANDALONE=TRUE
make -j
```

A static library will be generated: `libnoma_ocl.a`.

Execute the generated `oclinfo` tool:
```bash
./oclinfo
```
