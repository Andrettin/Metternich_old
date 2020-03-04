#pragma once

#include "script/modifier_effect/modifier_effect.h"
#include "util/number_util.h"

namespace metternich {

template <typename T>
class population_capacity_modifier_modifier_effect final : public modifier_effect<T>
{
public:
	population_capacity_modifier_modifier_effect(const int population_capacity_modifier)
		: population_capacity_modifier(population_capacity_modifier) {}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "population_capacity_modifier";
		return identifier;
	}

	virtual void apply(T *scope, const int change) const override
	{
		if constexpr (std::is_same_v<T, holding>) {
			scope->change_population_capacity_modifier(this->population_capacity_modifier * change);
		} else {
			scope->change_population_capacity_modifier(this->population_capacity_modifier * change);
		}
	}

	virtual std::string get_string() const override
	{
		return "Population Capacity Modifier: " + number::to_signed_string(this->population_capacity_modifier) + "%";
	}

private:
	int population_capacity_modifier = 0;
};

}
