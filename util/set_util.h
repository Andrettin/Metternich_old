#pragma once

#include <set>

namespace metternich::set {

template <typename T, typename U>
void merge(T &set, const U &other_container)
{
	for (const typename U::value_type &element : other_container) {
		set.insert(element);
	}
}

}
