#pragma once

#include <random>

namespace metternich {

class random
{
public:
	template <typename T = int>
	static T generate(const T modulo)
	{
		return random::generate_in_range<T>(static_cast<T>(0), modulo - static_cast<T>(1));
	}

	template <typename T = int>
	static T generate_in_range(const T min_value, const T max_value)
	{
		if constexpr (std::is_integral_v<T>) {
			std::uniform_int_distribution<T> distribution(min_value, max_value);
			return distribution(random::engine);
		} else {
			std::uniform_real_distribution<T> distribution(min_value, max_value);
			return distribution(random::engine);
		}
	}

	static double generate_angle();

private:
	static inline std::random_device random_device = std::random_device();
	static inline std::mt19937 engine = std::mt19937(random::random_device());
};

}
