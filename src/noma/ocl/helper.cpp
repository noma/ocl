// Copyright (c) 2015-2016 Matthias Noack <ma.noack.pr@gmail.com>
//
// See accompanying file LICENSE and README for further information.

#include "noma/ocl/helper.hpp"

#include <fstream>
#include <iostream>

#include "debug.hpp"
#include "noma/ocl/config_error.hpp"

namespace noma {
namespace ocl {

extern "C" CL_CALLBACK void cl_mem_destructor_callback(cl_mem, void* user_data)
{
	cl_mem_destructor_callback_info* info = reinterpret_cast<cl_mem_destructor_callback_info*>(user_data);
	//std::cout << "cl_mem_destructor_callback: " << info->ocl_helper().allocated_byte();
	info->ocl_helper().unregister_buffer(*info);
	//std::cout << " -> " << info->ocl_helper().allocated_byte() << std::endl;

	delete info; // cleanup, corresponding new is in buffer::buffer(..)
}

buffer::buffer(helper& h, cl_mem_flags flags, size_t size, void* host_ptr)
	: size_(size), mem_flags_(flags)
{
	cl_int err = 0;
	cl_buffer_ = cl::Buffer(h.context(), flags, size, host_ptr, &err); // NOTE: initialised here to check error C-style
	error_handler(err, "cl::Buffer()");

	// register a destructor callback for counting memory down when resources are freed
	auto info = new cl_mem_destructor_callback_info(h, size);
	h.register_buffer(*info);
	cl_buffer_.setDestructorCallback(cl_mem_destructor_callback, reinterpret_cast<void*>(info));
}

helper::helper(const config& config)
	: config_(config)
{
	cl_int err = CL_SUCCESS;
	
	// get available platforms
	std::vector<cl::Platform> platforms;
	err = cl::Platform::get(&platforms);
	error_handler(err, "cl::Platform::get()");

	// get a list of devices on this platform
	std::vector<cl::Device> device_list;
	err = platforms[config_.opencl_platform_index()].getDevices(CL_DEVICE_TYPE_ALL, &device_list);
	error_handler(err, "cl::Platform::getDevices()");

	// select device from config
	device_ = device_list[config_.opencl_device_index()];
	devices_.push_back(device_); // NOTE: just one device is used here
	err = device_.getInfo(CL_DEVICE_TYPE, &device_type_);
	error_handler(err, "cl::Device::getInfo()");

	// create context for devices select platform from config and create a context
	cl_context_properties cps[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[config_.opencl_platform_index()])(), 0 };
	context_ = cl::Context(devices_, cps, nullptr, nullptr, &err);
	error_handler(err, "cl::Context()");

	// create a command queue and use the device from config
	queue_ = cl::CommandQueue(context_, device_, CL_QUEUE_PROFILING_ENABLE, &err);
	error_handler(err, "cl::CommandQueue()");
}

cl::Program helper::create_program_from_file(const std::string& source_file_name, const std::string& source_header, const std::string& compile_options)
{
	std::ifstream source_file(source_file_name);
	if (source_file.fail())
		throw config_error("could not open source file: " + source_file_name);

	std::string source(std::istreambuf_iterator<char>(source_file), (std::istreambuf_iterator<char>()));
	return create_program(source, source_header, compile_options);
}

cl::Program helper::create_program(const std::string& source, const std::string& source_header, const std::string& compile_options)
{
	// prepend header_source
	std::string complete_source { source_header + "\n" + source };
	DEBUG_ONLY( std::cout << "helper::create_program(..): Prepending source header:\n" << source_header << std::endl; )
	return create_program(complete_source.c_str(), complete_source.length(), compile_options);
}

cl::Program helper::create_program(const char* source, size_t length, const std::string& compile_options)
{
	cl_int err = CL_SUCCESS;
	std::string build_info;

	cl::Program::Sources cl_src(1, std::make_pair(source, length)); // NOTE length can be 0 (for null-terminated c-strings), or > 0 exlucing any null-terminating character
	cl::Program prog(context_, cl_src, &err);
	error_handler(err, "cl::Program()");

	// prepend configuration file options
	std::string complete_compile_options = config_.opencl_compile_options() + " " + compile_options;

	DEBUG_ONLY( std::cout << "OpenCL: Building kernel with options: \"" << complete_compile_options << "\"" << std::endl; )
	err = prog.build(devices_, complete_compile_options.c_str());
	error_handler(err, "cl::Program::build()", false); // do not terminate, as we want the build log
	cl_int build_err = err;

	build_info = prog.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device_, &err);
	error_handler(err, "cl::Program::getBuildInfo()");

	std::string build_info_msg { "\n-----BEGIN BUILD LOG-----\n" + build_info + "\n-----END BUILD LOG-----" };

	if (build_err != CL_SUCCESS) {
		throw build_error(build_info_msg);
	} else {
		DEBUG_ONLY( std::cout << "OpenCL: Build log:" << build_info_msg << std::endl; )
	}


	return prog;
}

cl_ulong helper::run_kernel_timed(cl::Kernel& kernel, nd_range range)
{
	cl_int err = CL_SUCCESS;
	cl::Event event;
	cl_ulong t_start = 0;
	cl_ulong t_end = 0;

	// execute kernel
	queue_.enqueueNDRangeKernel(kernel, range.offset, range.global, range.local, nullptr, &event);
	error_handler(err, "ocl.queue().enqueueNDRangeKernel()");
	err = cl::Event::waitForEvents({event}); // NOTE: C++11 initialisation for an expected vector of events here
	error_handler(err, "cl::Event::waitForEvents()");

	// get times for statistics
	err = event.getProfilingInfo(CL_PROFILING_COMMAND_START, &t_start);
	error_handler(err, "event.getProfilingInfo()");
	err = event.getProfilingInfo(CL_PROFILING_COMMAND_END, &t_end);
	error_handler(err, "event.getProfilingInfo()");

	// return measured time
	return t_end - t_start;
}

void helper::write_device_info(std::ostream& os) const
{
	cl_int err = CL_SUCCESS;

	const std::string ind = "  "; // indentation
	std::string result;
	// output device info
	os << ind << "Platform " << config_.opencl_platform_index() << ": " << std::endl;
	os << ind << ind << "Device " << config_.opencl_device_index() << ": " << std::endl;

	err = device_.getInfo(CL_DEVICE_NAME, &result);
	error_handler(err, "cl::Device::getInfo()");
	os << ind << ind << ind << "CL_DEVICE_NAME: " << result << std::endl;

	err = device_.getInfo(CL_DEVICE_VENDOR, &result);
	error_handler(err, "cl::Device::getInfo()");
	os << ind << ind << ind << "CL_DEVICE_VENDOR: " << result << std::endl;

	err = device_.getInfo(CL_DEVICE_PROFILE, &result);
	error_handler(err, "cl::Device::getInfo()");
	os << ind << ind << ind << "CL_DEVICE_PROFILE: " << result << std::endl;

	err = device_.getInfo(CL_DEVICE_VERSION, &result);
	error_handler(err, "cl::Device::getInfo()");
	os << ind << ind << ind << "CL_DEVICE_VERSION: " << result << std::endl;

	err = device_.getInfo(CL_DRIVER_VERSION, &result);
	error_handler(err, "cl::Device::getInfo()");
	os << ind << ind << ind << "CL_DRIVER_VERSION: " << result << std::endl;

	err = device_.getInfo(CL_DEVICE_OPENCL_C_VERSION, &result);
	error_handler(err, "cl::Device::getInfo()");
	os << ind << ind << ind << "CL_DEVICE_OPENCL_C_VERSION: " << result << std::endl;

	err = device_.getInfo(CL_DEVICE_EXTENSIONS, &result);
	error_handler(err, "cl::Device::getInfo()");
	os << ind << ind << ind << "CL_DEVICE_EXTENSIONS: " << result << std::endl;
}

buffer helper::create_buffer(cl_mem_flags flags, size_t size, void* host_ptr)
{
	// allocate a zero_copy_buffer for CPU devices if no user-allocated memory was provided
	if (zero_copy_available() && !(flags & CL_MEM_USE_HOST_PTR))
	{
		flags |= CL_MEM_ALLOC_HOST_PTR;
	}
	return buffer(*this, flags, size, host_ptr);

	// NOTE: manual buffer management for zero-copy would look like this
	//       no significant performance difference on HLRN (< 3% on average), small sample
	//       => not worth struggling with manual de-allocation (ommitted below)
	//host_ptr = memory::aligned::instance().allocate_byte(size, 4096); // page size aligned memory
	//return buffer(context_, flags |= CL_MEM_USE_HOST_PTR, size, host_ptr);
}

void helper::register_buffer(const cl_mem_destructor_callback_info& info)
{
	add_allocation(info.size());
}

void helper::unregister_buffer(const cl_mem_destructor_callback_info& info)
{
	sub_allocation(info.size());
}

std::string error_to_string(cl_int error)
{
	// implementation from 
	// https://github.com/boostorg/compute
	// include/boost/compute/exception/opencl_error.hpp
	switch (error)
	{
	case CL_SUCCESS: return "Success";
	case CL_DEVICE_NOT_FOUND: return "Device Not Found";
	case CL_DEVICE_NOT_AVAILABLE: return "Device Not Available";
	case CL_COMPILER_NOT_AVAILABLE: return "Compiler Not Available";
	case CL_MEM_OBJECT_ALLOCATION_FAILURE: return "Memory Object Allocation Failure";
	case CL_OUT_OF_RESOURCES: return "Out of Resources";
	case CL_OUT_OF_HOST_MEMORY: return "Out of Host Memory";
	case CL_PROFILING_INFO_NOT_AVAILABLE: return "Profiling Information Not Available";
	case CL_MEM_COPY_OVERLAP: return "Memory Copy Overlap";
	case CL_IMAGE_FORMAT_MISMATCH: return "Image Format Mismatch";
	case CL_IMAGE_FORMAT_NOT_SUPPORTED: return "Image Format Not Supported";
	case CL_BUILD_PROGRAM_FAILURE: return "Build Program Failure";
	case CL_MAP_FAILURE: return "Map Failure";
	case CL_INVALID_VALUE: return "Invalid Value";
	case CL_INVALID_DEVICE_TYPE: return "Invalid Device Type";
	case CL_INVALID_PLATFORM: return "Invalid Platform";
	case CL_INVALID_DEVICE: return "Invalid Device";
	case CL_INVALID_CONTEXT: return "Invalid Context";
	case CL_INVALID_QUEUE_PROPERTIES: return "Invalid Queue Properties";
	case CL_INVALID_COMMAND_QUEUE: return "Invalid Command Queue";
	case CL_INVALID_HOST_PTR: return "Invalid Host Pointer";
	case CL_INVALID_MEM_OBJECT: return "Invalid Memory Object";
	case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: return "Invalid Image Format Descriptor";
	case CL_INVALID_IMAGE_SIZE: return "Invalid Image Size";
	case CL_INVALID_SAMPLER: return "Invalid Sampler";
	case CL_INVALID_BINARY: return "Invalid Binary";
	case CL_INVALID_BUILD_OPTIONS: return "Invalid Build Options";
	case CL_INVALID_PROGRAM: return "Invalid Program";
	case CL_INVALID_PROGRAM_EXECUTABLE: return "Invalid Program Executable";
	case CL_INVALID_KERNEL_NAME: return "Invalid Kernel Name";
	case CL_INVALID_KERNEL_DEFINITION: return "Invalid Kernel Definition";
	case CL_INVALID_KERNEL: return "Invalid Kernel";
	case CL_INVALID_ARG_INDEX: return "Invalid Argument Index";
	case CL_INVALID_ARG_VALUE: return "Invalid Argument Value";
	case CL_INVALID_ARG_SIZE: return "Invalid Argument Size";
	case CL_INVALID_KERNEL_ARGS: return "Invalid Kernel Arguments";
	case CL_INVALID_WORK_DIMENSION: return "Invalid Work Dimension";
	case CL_INVALID_WORK_GROUP_SIZE: return "Invalid Work Group Size";
	case CL_INVALID_WORK_ITEM_SIZE: return "Invalid Work Item Size";
	case CL_INVALID_GLOBAL_OFFSET: return "Invalid Global Offset";
	case CL_INVALID_EVENT_WAIT_LIST: return "Invalid Event Wait List";
	case CL_INVALID_EVENT: return "Invalid Event";
	case CL_INVALID_OPERATION: return "Invalid Operation";
	case CL_INVALID_GL_OBJECT: return "Invalid GL Object";
	case CL_INVALID_BUFFER_SIZE: return "Invalid Buffer Size";
	case CL_INVALID_MIP_LEVEL: return "Invalid MIP Level";
	case CL_INVALID_GLOBAL_WORK_SIZE: return "Invalid Global Work Size";
	#ifdef CL_VERSION_1_2
	case CL_COMPILE_PROGRAM_FAILURE: return "Compile Program Failure";
	case CL_LINKER_NOT_AVAILABLE: return "Linker Not Available";
	case CL_LINK_PROGRAM_FAILURE: return "Link Program Failure";
	case CL_DEVICE_PARTITION_FAILED: return "Device Partition Failed";
	case CL_KERNEL_ARG_INFO_NOT_AVAILABLE: return "Kernel Argument Info Not Available";
	case CL_INVALID_PROPERTY: return "Invalid Property";
	case CL_INVALID_IMAGE_DESCRIPTOR: return "Invalid Image Descriptor";
	case CL_INVALID_COMPILER_OPTIONS: return "Invalid Compiler Options";
	case CL_INVALID_LINKER_OPTIONS: return "Invalid Linker Options";
	case CL_INVALID_DEVICE_PARTITION_COUNT: return "Invalid Device Partition Count";
	#endif // CL_VERSION_1_2
	#ifdef CL_VERSION_2_0
	case CL_INVALID_PIPE_SIZE: return "Invalid Pipe Size";
	case CL_INVALID_DEVICE_QUEUE: return "Invalid Device Queue";
	#endif
	default:
	{
		std::stringstream s;
		s << "Unknown OpenCL Error (" << error << ")";
		return s.str();
	}
	}
}

bool error_handler(cl_int error, const std::string& function_name, bool throw_exception)
{
	if (error != CL_SUCCESS)
	{

		std::string msg { "OpenCL error: " + function_name + " returned " + error_to_string(error) };
		if (throw_exception)
			throw std::runtime_error(msg);
		else
			std::cerr << msg << std::endl;

		return true;
	}
	else
	{
		return false; // no error
	}
}

} // namespace ocl
} // namespace noma
