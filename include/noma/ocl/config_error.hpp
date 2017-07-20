// Copyright (c) 2015-2017 Matthias Noack <ma.noack.pr@gmail.com>
//
// See accompanying file LICENSE and README for further information.

#ifndef noma_ocl_config_error_hpp
#define noma_ocl_config_error_hpp

#include <stdexcept>
#include <string>

namespace noma {
namespace ocl {

/**
 * Custom exception class for ocl configuration errors.
 */
class config_error : public std::runtime_error
{
public:
	config_error(const std::string& msg) : runtime_error("ocl::config_error: " + msg) { };
};

} // namespace ocl
} // namespace noma

#endif // noma_ocl_config_error_hpp
