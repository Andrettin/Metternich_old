#pragma once

#include <string>

namespace metternich::parse {

/**
**	@brief	Convert a fractional number string (i.e. a number with up to two decimal places) to an integer, multiplying it by 10 ^ the number of decimal places; the template argument is the number of decimal places
**
**	@param	str	The string
**
**	@return	The integer
*/
template <int N>
inline int fractional_number_string_to_int(const std::string &str)
{
	size_t decimal_point_pos = str.find('.');
	int integer = 0;
	int fraction = 0;
	if (decimal_point_pos != std::string::npos) {
		integer = std::stoi(str.substr(0, decimal_point_pos));
		const size_t decimal_pos = decimal_point_pos + 1;
		const size_t decimal_places = str.length() - decimal_pos;
		fraction = std::stoi(str.substr(decimal_pos, decimal_places));
		if (decimal_places < N) {
			for (int i = static_cast<int>(decimal_places); i < N; ++i) {
				fraction *= 10;
			}
		}
		const bool negative = str.front() == '-';
		if (negative) {
			fraction *= -1;
		}
	} else {
		integer = std::stoi(str);
	}

	for (int i = 0; i < N; ++i) {
		integer *= 10;
	}
	integer += fraction;

	return integer;
}

inline int centesimal_number_string_to_int(const std::string &str)
{
	return fractional_number_string_to_int<2>(str);
}

inline int millesimal_number_string_to_int(const std::string &str)
{
	return fractional_number_string_to_int<3>(str);
}

}
