// Copyright (c) 2015-2016 Matthias Noack <ma.noack.pr@gmail.com>
//
// See accompanying file LICENSE and README for further information.

#ifndef noma_ocl_helper_hpp
#define noma_ocl_helper_hpp

#include <string>

#include <CL/cl.hpp>

#include "common.hpp"
#include "memory.hpp"
#include "noma/ocl/config.hpp"

namespace noma {
namespace ocl {

/**
 * Custom exception class for configuration errors.
 */
class build_error : public std::runtime_error
{
public:
	build_error(const std::string& msg) : std::runtime_error("ocl::build_error: " + msg) { };
};

struct nd_range
{
	cl::NDRange offset;
	cl::NDRange global;
	cl::NDRange local;
};

class helper; // forward decl

/**
 * Callback function to pass to OpenCL when memory resources are freed, to keep count.
 * The user_data is used to pass a piece of heap memory.
 * This function must be thread-safe and can be called asynchronously from within the
 * OpenCL runtime.
 */
extern "C" CL_CALLBACK void cl_mem_destructor_callback(cl_mem, void* user_data);

/**
 * Class to be passed as user data to cl_mem_destructor_callback.
 */
class cl_mem_destructor_callback_info
{
public:
	cl_mem_destructor_callback_info(helper& ocl_helper, size_t size) : ocl_helper_(ocl_helper), size_(size) {}
	size_t size() const { return size_; }
	helper& ocl_helper() const { return ocl_helper_; }
private:
	helper& ocl_helper_;
	size_t size_ = 0;
};

/**
 * Wraps a cl::Buffer object to add some additional properties
 */
class buffer {
public:
	buffer(helper& h, cl_mem_flags flags, size_t size, void* host_ptr = nullptr);

	operator cl::Buffer&() { return cl_buffer_;	} // NOTE: implicit conversion possible
	size_t size() const { return size_; }
	bool is_zero_copy() const { return mem_flags_ & CL_MEM_ALLOC_HOST_PTR; }

private:
	cl::Buffer cl_buffer_;
	size_t size_;
	cl_mem_flags mem_flags_;
};

class helper : public memory::allocation_counter {
public:
	helper(const config& config);

	cl::Context& context() { return context_; }
	cl::Device& device() { return device_; }
	cl_device_type device_type() { return device_type_; }
	cl::CommandQueue& queue() { return queue_; }
	
	cl::Program create_program_from_file(const std::string& source_file_name, const std::string& source_header, const std::string& compile_options);
	cl::Program create_program(const std::string& source, const std::string& source_header, const std::string& compile_options);
	cl::Program create_program(const char* source, size_t length, const std::string& compile_options);

	cl_ulong run_kernel_timed(cl::Kernel& kernel, nd_range range);

	bool zero_copy_available() const { return config_.opencl_zero_copy_device_types() & device_type_; }

	buffer create_buffer(cl_mem_flags, size_t size, void* host_ptr = nullptr);

	// count allocations for statistics
	void register_buffer(const cl_mem_destructor_callback_info& info);
	void unregister_buffer(const cl_mem_destructor_callback_info& info);

	void write_device_info(std::ostream& os) const;
private:
	const config& config_;
	cl::Context context_;
	std::vector<cl::Device> devices_; // the list of used devices
	cl::Device device_; // NOTE: this is the single device currently used
	cl_device_type device_type_;
	cl::CommandQueue queue_;
};

std::string error_to_string(cl_int error);

bool error_handler(cl_int err, const std::string& function_name, bool throw_exception = true);

} // namespace ocl
} // namespace noma

#endif // noma_ocl_helper_hpp
