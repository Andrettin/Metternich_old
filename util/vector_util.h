#pragma once

#include <vector>

namespace metternich::vector {

template <typename T, typename U>
bool contains(const std::vector<T> &vector, const U &value)
{
	return std::find(vector.begin(), vector.end(), value) != vector.end();
}

template <typename T, typename U>
void merge(std::vector<T> &vector, const U &other_container)
{
	for (const typename U::value_type &element : other_container) {
		vector.push_back(element);
	}
}

template <typename T, typename U>
void merge(std::vector<T> &vector, U &&other_container)
{
	for (typename U::value_type &element : other_container) {
		vector.push_back(std::move(element));
	}
}

template <typename T>
inline void remove(T &vector, const typename T::value_type &element)
{
	vector.erase(std::remove(vector.begin(), vector.end(), element), vector.end());
}

template <typename T>
inline void remove_one(T &vector, const typename T::value_type &element)
{
	vector.erase(std::find(vector.begin(), vector.end(), element));
}

}
