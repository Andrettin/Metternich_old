#pragma once

#include <vector>

namespace metternich::vector {

template <typename T, typename U>
void merge(std::vector<T> &container, const U &other_container)
{
	for (const typename U::value_type &element : other_container) {
		container.push_back(element);
	}
}

}
