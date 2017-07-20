// Copyright (c) 2015-2017 Matthias Noack <ma.noack.pr@gmail.com>
//
// See accompanying file LICENSE and README for further information.

#ifndef noma_ocl_config_hpp
#define noma_ocl_config_hpp

#include <string>

#include <boost/program_options.hpp>
#include <CL/cl.hpp>

#include "noma/ocl/device_type_list.hpp"

namespace noma {
namespace ocl {

/**
 * OpenCL configuration file used by ocl::helper.
 */
class config
{
public:
	config(const std::string& config_file_name, bool parse_config = true);

	void help(std::ostream& out) const;

	// config value getters
	const size_t& opencl_platform_index() const
	{ return opencl_platform_index_; }

	const size_t& opencl_device_index() const
	{ return opencl_device_index_; }

	cl_device_type opencl_zero_copy_device_types() const
	{ return opencl_zero_copy_device_types_; }

	const std::string& opencl_compile_options() const
	{ return opencl_compile_options_; }

	// TODO: add print to output config file? (to all config file classes)
protected:
	void parse(const std::string& config_file_name);

	boost::program_options::options_description desc_;

private:
	// config values
	size_t opencl_platform_index_;
	size_t opencl_device_index_;

	device_type_list opencl_zero_copy_device_types_;

	std::string opencl_compile_options_;
	//CPU: " -cl-mad-enable -auto-prefetch-level=" STR(INTEL_PREFETCH_LEVEL) " ";
	//ACC: " -cl-mad-enable -auto-prefetch-level=" STR(INTEL_PREFETCH_LEVEL) " "; // -cl-finite-math-only -cl-no-signed-zeros ";
	//GPU: -cl-nv-verbose -cl-nv-opt-level=3 -cl-mad-enable -cl-strict-aliasing -cl-nv-arch sm_35 -cl-nv-maxrregcount=64 ";

	// TODO: maybe add work-group size
};

} // namespace ocl
} // namespace noma

#endif // noma_ocl_config_hpp
