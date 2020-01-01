#pragma once

#include <set>
#include <vector>

namespace metternich::map {

template <typename T>
inline std::set<typename T::key_type> get_keys(const T &map)
{
	std::set<typename T::key_type> set;

	for (const auto &kv_pair : map) {
		set.insert(kv_pair.first);
	}

	return set;
}

template <typename T>
inline std::vector<typename T::mapped_type> get_values(const T &map)
{
	std::vector<typename T::mapped_type> vector;

	for (const auto &kv_pair : map) {
		vector.push_back(kv_pair.second);
	}

	return vector;
}

}
