// Copyright (c) 2015-2017 Matthias Noack <ma.noack.pr@gmail.com>
//
// See accompanying file LICENSE and README for further information.

#ifndef noma_ocl_device_type_list_hpp
#define noma_ocl_device_type_list_hpp

#include <iostream>
#include <string>

#include <CL/cl.hpp>

namespace noma {
namespace ocl {

/**
 * Parsable wrapper type that adds a braced list text format to the
 * cl_device_type C-bitfield type.
 */
class device_type_list
{
public:
	device_type_list() = default;
	device_type_list(const cl_device_type type) : type_(type) {}
	operator cl_device_type() const { return type_; }
private:
	cl_device_type type_ = 0;
};

std::ostream& operator<<(std::ostream& out, const device_type_list& t);
std::istream& operator>>(std::istream& in, device_type_list& t);

/**
 * String representations for OpenCL's cl_device_type
 */
const std::map<device_type_list, std::string> cl_device_type_names {
	{ 0, "none" },
	{ CL_DEVICE_TYPE_ALL, "all" },
	{ CL_DEVICE_TYPE_CPU, "cpu" },
	{ CL_DEVICE_TYPE_ACCELERATOR, "accelerator" },
	{ CL_DEVICE_TYPE_GPU, "gpu" }};

} // namespace ocl
} // namespace noma

#endif // noma_ocl_device_type_list_hpp
