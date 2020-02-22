#pragma once

namespace metternich::number {

/**
**	@brief	Round a double to a certain number of decimal places
**
**	@param	number	The number
**
**	@return	The rounded number
*/
template <int N>
inline double round_to_decimal_places(double number)
{
	for (int i = 0; i < N; ++i) {
		number *= 10.0;
	}

	number = std::round(number);

	for (int i = 0; i < N; ++i) {
		number /= 10.0;
	}

	return number;
}

inline double round_to_decimal(double number)
{
	return round_to_decimal_places<1>(number);
}

inline double round_to_centesimal(double number)
{
	return round_to_decimal_places<2>(number);
}

/**
**	@brief	Floor a double to a certain number of decimal places
**
**	@param	number	The number
**
**	@return	The floored number
*/
template <int N>
inline double floor_to_decimal_places(double number)
{
	for (int i = 0; i < N; ++i) {
		number *= 10.0;
	}

	number = std::floor(number);

	for (int i = 0; i < N; ++i) {
		number /= 10.0;
	}

	return number;
}

inline double floor_to_decimal(double number)
{
	return floor_to_decimal_places<1>(number);
}

inline double floor_to_centesimal(double number)
{
	return floor_to_decimal_places<2>(number);
}

inline std::string to_signed_string(const int number)
{
	std::string number_str;
	if (number >= 0) {
		number_str += "+";
	}
	number_str += std::to_string(number);
	return number_str;
}

inline std::string to_centesimal_rest_string(const int rest)
{
	std::string rest_str;
	const int abs_rest = std::abs(rest);
	if (abs_rest > 0) {
		rest_str += ".";
		if (abs_rest < 10) {
			rest_str += "0";
		}
		rest_str += std::to_string(abs_rest);
	}
	return rest_str;
}

inline std::string to_centesimal_string(const int number)
{
	std::string number_str = std::to_string(number / 100);
	number_str += number::to_centesimal_rest_string(number % 100);
	return number_str;
}

inline std::string to_signed_centesimal_string(const int number)
{
	std::string number_str = number::to_signed_string(number / 100);
	number_str += number::to_centesimal_rest_string(number % 100);
	return number_str;
}

extern double degree_to_radian(const double degree);

}
