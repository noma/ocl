// Copyright (c) 2015-2017Matthias Noack <ma.noack.pr@gmail.com>
//
// See accompanying file LICENSE and README for further information.

#include "noma/ocl/config.hpp"

#include <noma/typa/typa.hpp>

#include "debug.hpp"

namespace noma {
namespace ocl {

namespace bpo = ::boost::program_options;

std::ostream& operator<<(std::ostream& out, const device_type_list& t)
{
	bool first = true;
	out << '{';
	if (t == 0 || t == CL_DEVICE_TYPE_ALL) {
		out << cl_device_type_names.at(t);
	} else {
		if (t & CL_DEVICE_TYPE_CPU) {
			out << (!first ? "," : "") << cl_device_type_names.at(CL_DEVICE_TYPE_CPU);
			first = false;
		}
		if (t & CL_DEVICE_TYPE_ACCELERATOR) {
			out << (!first ? "," : "") << cl_device_type_names.at(CL_DEVICE_TYPE_ACCELERATOR);
			first = false;
		}
		if (t & CL_DEVICE_TYPE_GPU) {
			out << (!first ? "," : "") << cl_device_type_names.at(CL_DEVICE_TYPE_GPU);
			first = false;
		}
	}
	out << '}';
	return out;
}

std::istream& operator>>(std::istream& in, device_type_list& t)
{
	std::string value;
	std::getline(in, value);

	// manually parse opencl_zero_copy_device_types_string_ into opencl_zero_copy_device_types_
	std::string name_exp_str;
	for (auto& name : cl_device_type_names) {
		if (!name_exp_str.empty())
			name_exp_str += "|";
		name_exp_str += name.second;
	}
	name_exp_str = "(?:" + name_exp_str + ")";

	std::string input_no_ws { noma::typa::remove_whitespace(value) };
	const std::regex entry_exp { name_exp_str };
	const std::regex list_exp { noma::typa::make_braced_list(name_exp_str) };

	if(!std::regex_match(input_no_ws, list_exp))
		throw noma::typa::parser_error("ocl::device_type_list::operator>>(): error: malformed input, should be comma separated braced list, with values from: " + noma::typa::get_map_value_list_string(cl_device_type_names));

	cl_device_type result = 0;

	std::smatch sm;
	std::string::const_iterator cit = input_no_ws.cbegin();
	while (std::regex_search(cit, input_no_ws.cend(), sm, entry_exp)) {
		//std::cout << "#Matches: " << sm.size() << std::endl;
		for (auto& m : sm) {
			ASSERT_ONLY( bool found = false; )
			for (auto it = cl_device_type_names.begin(); it != cl_device_type_names.end(); ++it)
				if (it->second == m.str()) {
					result |= it->first;
					ASSERT_ONLY( found = true; )
					break;
				}
			assert(found); // since we build the regex from the map, the value should be in there
		}
		cit = sm[0].second; // start of remaining string
	}

	t = result;
	DEBUG_ONLY( std::cout << "ocl::device_type_list::operator>>(): parsed: " << t << ", as cl_device_type = " << result << std::endl; )

	return in;
}

} // namespace ocl
} // namespace noma
