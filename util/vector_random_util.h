#pragma once

#include "random.h"

#include <vector>

namespace metternich::vector {

template <typename T>
inline const typename T::value_type get_random(const T &vector)
{
	return vector[random::generate(vector.size())];
}

}
