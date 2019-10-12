#pragma once

namespace metternich::util {

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

}
