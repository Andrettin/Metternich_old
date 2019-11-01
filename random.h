#pragma once

#include <random>

namespace metternich {

/**
**	@brief	Random number generator
*/
class random
{
public:
	template <typename T = int>
	static T generate(const T modulo)
	{
		return random::generate_in_range(static_cast<T>(0), modulo - 1);
	}

	template <typename T = int>
	static T generate_in_range(const T min_value, const T max_value)
	{
		std::uniform_int_distribution<T> distribution(min_value, max_value);
		return distribution(random::random_device);
	}

private:
	static inline std::random_device random_device = std::random_device();
};

}
