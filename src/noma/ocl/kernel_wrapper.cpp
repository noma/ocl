// Copyright (c) 2016 Matthias Noack <ma.noack.pr@gmail.com>
//
// See accompanying file LICENSE and README for further information.

#include "noma/ocl/kernel_wrapper.hpp"
#include "debug.hpp"

namespace noma {
namespace ocl {

kernel_wrapper::kernel_wrapper(helper& ocl)
	: ocl_(ocl),
	  kernel_file_name_(""),
	  kernel_name_(""),
	  range_()
{
	// nothing to do here
}

kernel_wrapper::kernel_wrapper(helper& ocl, const std::string& kernel_source, const std::string& kernel_name, const std::string& source_header, const std::string& compile_option, const nd_range& range)
	: ocl_(ocl),
	  kernel_file_name_(""),
	  kernel_name_(kernel_name),
	  range_(range)
{
	cl_int err = 0;

	// build kernel
	kernel_ = cl::Kernel(ocl.create_program(kernel_source, source_header, compile_option),
	                    kernel_name_.c_str(), &err);
	error_handler(err, "building kernel '" + kernel_name_ + "' from source string failed");

	//set_static_args(static_args);
}

kernel_wrapper::kernel_wrapper(helper& ocl, const boost::filesystem::path& kernel_file_path, const std::string& kernel_name, const std::string& source_header, const std::string& compile_option, const nd_range& range)
	: ocl_(ocl),
	  kernel_file_name_(kernel_file_path.string()),
	  kernel_name_(kernel_name),
	  range_(range)
{
	cl_int err = 0;

	// build kernel
	kernel_ = cl::Kernel(ocl.create_program_from_file(kernel_file_name_, source_header, compile_option),
	                    kernel_name_.c_str(), &err);
	error_handler(err, "building kernel '" + kernel_name_ + "' from source file '" + kernel_file_name_ + "' failed");

	//set_static_args(static_args);
}

void kernel_wrapper::run_kernel()
{
	if (kernel_name_.empty())
		throw std::runtime_error("ocl::kernel_wrapper::run(): error: called on dummy-initialised instance.");

	// run and benchmark kernel
	DEBUG_ONLY( std::cout << "kernel_wrapper::run_kernel(): " << kernel_name_ << std::endl; )
	stats_.add(noma::bmt::duration(static_cast<noma::bmt::rep>(ocl_.run_kernel_timed(kernel_, range_))));
}

//template<class STATIC_ARG_SET_T, class DYNAMIC_ARG_SET_T>
//void kernel_wrapper<STATIC_ARG_SET_T, DYNAMIC_ARG_SET_T>::run()
//{
//
//
//};
//
//template<class STATIC_ARG_SET_T, class DYNAMIC_ARG_SET_T>
//void kernel_wrapper<STATIC_ARG_SET_T, DYNAMIC_ARG_SET_T>::set_dynamic_args(const DYNAMIC_ARG_SET_T& dynamic_args)
//{
//
//};

} // namespace ocl
} // namespace noma
