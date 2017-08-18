# Copyright (c) 2015-2017 Matthias Noack <ma.noack.pr@gmail.com>
#
# See accompanying file LICENSE and README for further information.

# Usage example: see noma::num library

if(__create_opencl_kernel_header)
	return()
endif()
set(__create_opencl_kernel_header YES)

set(CWD_create_opencl_kernel_header "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "" FORCE)

# we need gawk for resolve_includes.sh
set(AWK_NAME gawk)
find_program(AWK_BIN NAMES ${AWK_NAME})

if(AWK_BIN STREQUAL "AWK_BIN-NOTFOUND")
	message(FATAL_ERROR "'${AWK_NAME}' not found.")
else()
	message(STATUS "Found ${AWK_NAME}: ${AWK_BIN}")
endif()

# TODO: include directives within OpenCL are resolved, but no dependencies within CMake are created, i.e. changing included files within OpenCL kernels requires a make clean to trigger re-generation of kernel code C++ headers
# maybe write script to extract includes from OpenCL code, or re-use CMake functionality

# OpenCL kernels are compiled into the application
# the script "cl_to_header.sh is used to convert each OpenCL code file into a header file containing a charcter array and its length which contains the source code
# prior to that a copy of the kernel is made, renamed to have a .c ending and preprocessed with "resolve_includes.sh"

# Arguments: 
# KERNEL_FILE .. path to OpenCL kernel file (.cl)
# OpenCL_KERNEL_HEADER_DIR .. path to where the generated files will be placed
# KERNEL_HEADER_FILE_OUT .. exported variable name for dependency creation
function(create_opencl_kernel_header KERNEL_FILE OpenCL_KERNEL_HEADER_DIR KERNEL_HEADER_FILE_OUT)
#message( "Current dir: ${CWD_create_opencl_kernel_header}" )
	get_filename_component(KERNEL_DIR ${KERNEL_FILE} DIRECTORY)
	get_filename_component(KERNEL_BASENAME ${KERNEL_FILE} NAME_WE)

	set(KERNEL_FILE_PREPROCESSED ${OpenCL_KERNEL_HEADER_DIR}/${KERNEL_BASENAME}.cl) # use .cl ending for the result, NOTE: filename determines the name of string constants in generated header
	set(KERNEL_HEADER_FILE ${OpenCL_KERNEL_HEADER_DIR}/${KERNEL_BASENAME}.cl.hpp)
	#message("${KERNEL_HEADER_FILE}")

	add_custom_command(
		OUTPUT ${KERNEL_HEADER_FILE}
		COMMAND ${CWD_create_opencl_kernel_header}/resolve_includes.sh ${AWK_BIN} ${KERNEL_FILE} ${KERNEL_DIR} ${KERNEL_FILE_PREPROCESSED} # resolve include directives within the OpenCL code
		COMMAND ${CWD_create_opencl_kernel_header}/cl_to_header.sh ${KERNEL_FILE_PREPROCESSED} ${KERNEL_HEADER_FILE}
		WORKING_DIRECTORY ${KERNEL_DIR}
		COMMENT "Generating kernel header for ${KERNEL_FILE}"
		DEPENDS ${KERNEL_FILE}
	)

	# NOTE: we use the passed argument as name for a new variable we put into the caller's scope
	set(${KERNEL_HEADER_FILE_OUT} ${KERNEL_HEADER_FILE} PARENT_SCOPE)
endfunction(create_opencl_kernel_header)

