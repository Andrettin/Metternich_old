#pragma once

#include "script/modifier_effect/modifier_effect.h"

namespace metternich {

template <typename T>
class population_growth_modifier_effect : public modifier_effect<T>
{
public:
	population_growth_modifier_effect(const int population_growth)
		: population_growth(population_growth) {}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "population_growth";
		return identifier;
	}

	virtual void apply(T *scope, const int change) const override
	{
		if constexpr (std::is_same_v<T, holding>) {
			scope->change_base_population_growth(this->population_growth * change);
		} else {
			scope->change_population_growth_modifier(this->population_growth * change);
		}
	}

private:
	int population_growth = 0;
};

}
