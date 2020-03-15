#pragma once

#include "script/chance_factor.h"
#include "script/context.h"
#include "util/random.h"

#include <map>
#include <utility>

namespace metternich {

template <typename T>
class chance_factor;

template <typename T, typename U, typename V>
inline T calculate_chance_list_result(const std::map<T, const chance_factor<U> *> &chance_map, V *scope)
{
	std::map<T, std::pair<int, int>> chance_ranges;

	read_only_context ctx;
	if constexpr (std::is_same_v<V, character>) {
		ctx.current_character = scope;
	}

	int total_chance_factor = 0;
	for (const auto &kv_pair : chance_map) {
		const chance_factor<U> *chance_factor = kv_pair.second;
		const int chance = chance_factor->calculate(scope, ctx);
		if (chance > 0) {
			chance_ranges[kv_pair.first] = std::pair<int, int>(total_chance_factor, total_chance_factor + chance);
			total_chance_factor += chance;
		}
	}

	const int random_number = random::generate(total_chance_factor);
	for (const auto &kv_pair : chance_ranges) {
		const T &value = kv_pair.first;
		const std::pair<int, int> range = kv_pair.second;
		if (random_number >= range.first && random_number < range.second) {
			return value;
		}
	}

	throw std::runtime_error("No valid chance value when calculating the result of a chance list.");
}

}
