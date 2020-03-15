#pragma once

#include <type_traits>

namespace metternich {

class holding;
class holding_slot;
class landed_title;
class population_unit;
class province;
class territory;
class world;

template <typename T>
const territory *get_scope_territory(const T *scope)
{
	const territory *territory = nullptr;
	if constexpr (std::is_base_of_v<territory, T>) {
		territory = scope;
	} else {
		territory = scope->get_territory();
	}
	return territory;
}

template <typename T>
const province *get_scope_province(const T *scope)
{
	const province *province = nullptr;
	if constexpr (std::is_same_v<T, metternich::province>) {
		province = scope;
	} else {
		province = scope->get_province();
	}
	return province;
}

template <typename T>
const world *get_scope_world(const T *scope)
{
	const world *world = nullptr;
	if constexpr (std::is_same_v<T, metternich::world>) {
		world = scope;
	} else {
		world = scope->get_world();
	}
	return world;
}

template <typename T>
const landed_title *get_scope_landed_title(const T *scope)
{
	const landed_title *title = nullptr;
	if constexpr (std::is_same_v<T, metternich::province>) {
		title = scope->get_county();
	} else if constexpr (std::is_same_v<T, metternich::holding> || std::is_same_v<T, metternich::holding_slot>) {
		title = scope->get_barony();
	} else if constexpr (std::is_same_v<T, metternich::population_unit>) {
		title = scope->get_holding()->get_barony();
	} else {
		title = scope;
	}
	return title;
}

}
