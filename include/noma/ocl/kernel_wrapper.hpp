// Copyright (c) 2016 Matthias Noack <ma.noack.pr@gmail.com>
//
// See accompanying file LICENSE and README for further information.

#ifndef noma_ocl_kernel_wrapper_hpp
#define noma_ocl_kernel_wrapper_hpp

#include <string>

#include <boost/filesystem.hpp>
#include <noma/bmt/bmt.hpp>

#include "noma/ocl/helper.hpp"

namespace noma {
namespace ocl {

class kernel_wrapper
{
public:
	kernel_wrapper(helper& ocl); // for classes that do not actually use the an OpenCL kernel but are expected to have the same interface/concept
	kernel_wrapper(helper& ocl, const std::string& kernel_source, const std::string& kernel_name, const std::string& source_header, const std::string& compile_option, const nd_range& range);
	kernel_wrapper(helper& ocl, const boost::filesystem::path& kernel_file_path, const std::string& kernel_name, const std::string& source_header, const std::string& compile_option, const nd_range& range);

	void run_kernel();

	// TODO(maybe): this needs some template magic to work: two lists of tuples of pairs of cl_uint (argument index) and some T (argument type)
	//void set_static_args(const arg_list& args);
	//void set_dynamic_args(const arg_list& args);

	ocl::helper& ocl_helper() { return ocl_; }

	bool uses_kernel_file() const { return !kernel_file_name_.empty(); } // NOTE: non-empty means a kernel file was used, not a string
	const std::string& kernel_file_name() const { return kernel_file_name_; }

	const noma::bmt::statistics& kernel_stats() const { return stats_; }

protected:
	// OpenCL stuff
	helper& ocl_;
	const std::string kernel_file_name_;
	const std::string kernel_name_;
	cl::Kernel kernel_;
	const nd_range range_;

	// for benchmarking
	noma::bmt::statistics stats_;
};

} // namespace ocl
} // namespace noma

#endif // noma_ocl_kernel_wrapper_hpp
