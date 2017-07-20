// Copyright (c) 2015-2017Matthias Noack <ma.noack.pr@gmail.com>
//
// See accompanying file LICENSE and README for further information.

#include "noma/ocl/config.hpp"

#include <fstream>

#include <noma/typa/typa.hpp>

#include "common.hpp"
#include "debug.hpp"
#include "noma/ocl/config_error.hpp"

namespace noma {
namespace ocl {

namespace bpo = ::boost::program_options;

config::config(const std::string& config_file_name, bool parse_config)
	: desc_("OpenCL options"),
	  opencl_platform_index_(0),
	  opencl_device_index_(0),
	  opencl_zero_copy_device_types_(0)
{
	std::stringstream opencl_zero_copy_device_types_help;
	opencl_zero_copy_device_types_help << "Comma-separated, braced list of device types for which OpenCL buffers may be allocated as zero-copy buffers, and used with map/unmap instead of read/write. Allowed values: ";
	noma::typa::write_map_value_list(cl_device_type_names, opencl_zero_copy_device_types_help);

	// initialise program options description
	desc_.add_options()
		("opencl.platform_index", bpo::value(&opencl_platform_index_)->default_value(opencl_platform_index_), "Index of the OpenCL platform to use, in the array of available ones.")
		("opencl.device_index", bpo::value(&opencl_device_index_)->default_value(opencl_device_index_), "Index of the OpenCL device to use, in the array of available ones.")
		("opencl.compile_options", bpo::value(&opencl_compile_options_)->default_value(opencl_compile_options_), "OpenCL compilations options, prepended to all kernel compilations in addition to what is passed through the ocl::helper API.")
		("opencl.zero_copy_device_types", bpo::value(&opencl_zero_copy_device_types_)->default_value(opencl_zero_copy_device_types_), opencl_zero_copy_device_types_help.str().c_str())
	;

	if (parse_config)
		parse(config_file_name);
}

void config::parse(const std::string& config_file_name)
{
	DEBUG_ONLY( std::cout << "ocl::config::parse(): parsing OpenCL configuration from file: " << config_file_name << std::endl; )
	try {
		std::ifstream config_file(config_file_name);
		if (config_file.fail())
			throw config_error("could not open config file: " + config_file_name);

		bpo::variables_map vm;
		// true argument enables allow_unregistered options
		bpo::store(bpo::parse_config_file(config_file, desc_, true), vm);
		notify(vm);
	} catch (const config_error& e) {
		throw e;
	} catch (const bpo::invalid_option_value& e) {
		throw config_error(std::string("malformed value: ") + e.what());
	} catch (const std::exception& e) {
		throw config_error(e.what());
	}
}

void config::help(std::ostream& out) const
{
	out << desc_ << std::endl;
}

} // namespace ocl
} // namespace noma
