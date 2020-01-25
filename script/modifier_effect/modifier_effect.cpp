#include "script/modifier_effect/modifier_effect.h"

#include "character/character.h"
#include "database/gsml_property.h"
#include "holding/holding.h"
#include "map/province.h"
#include "script/modifier_effect/dueling_modifier_effect.h"
#include "script/modifier_effect/population_capacity_modifier_effect.h"
#include "script/modifier_effect/population_capacity_modifier_modifier_effect.h"
#include "script/modifier_effect/population_growth_modifier_effect.h"
#include "util/parse_util.h"

namespace metternich {

template <typename T>
std::unique_ptr<modifier_effect<T>> modifier_effect<T>::from_gsml_property(const gsml_property &property)
{
	const std::string &identifier = property.get_key();

	if constexpr (std::is_same_v<T, holding> || std::is_same_v<T, province>) {
		if (identifier == "population_capacity") {
			return std::make_unique<population_capacity_modifier_effect<T>>(std::stoi(property.get_value()));
		} else if (identifier == "population_capacity_modifier") {
			return std::make_unique<population_capacity_modifier_modifier_effect<T>>(parse::centesimal_number_string_to_int(property.get_value()));
		} else if (identifier == "population_growth") {
			return std::make_unique<population_growth_modifier_effect<T>>(parse::fractional_number_string_to_int<4>(property.get_value()));
		}
	}

	if constexpr (std::is_same_v<T, character>) {
		if (identifier == "dueling") {
			return std::make_unique<dueling_modifier_effect<T>>(std::stoi(property.get_value()));
		}

	}

	throw std::runtime_error("Invalid modifier effect: \"" + identifier + "\".");
}

template class modifier_effect<character>;
template class modifier_effect<holding>;
template class modifier_effect<province>;

}
