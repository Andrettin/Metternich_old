#pragma once

#include <random>

namespace metternich {

/**
**	@brief	Random number generator
*/
class Random
{
public:
	template <typename T = int>
	static T Generate(const T modulo)
	{
		return Random::GenerateInRange(static_cast<T>(0), modulo - 1);
	}

	template <typename T = int>
	static T GenerateInRange(const T min_value, const T max_value)
	{
		std::uniform_int_distribution<T> distribution(min_value, max_value);
		return distribution(Random::RandomDevice);
	}

private:
	static inline std::random_device RandomDevice = std::random_device();
};

}
