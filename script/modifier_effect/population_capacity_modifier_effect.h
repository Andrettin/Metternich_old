#pragma once

#include "holding/holding.h"
#include "map/province.h"
#include "script/modifier_effect/modifier_effect.h"

namespace metternich {

/**
**	@brief	A population capacity modifier effect
*/
class population_capacity_modifier_effect : public modifier_effect
{
public:
	population_capacity_modifier_effect(const int population_capacity) : population_capacity(population_capacity) {}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "population_capacity";
		return identifier;
	}

	virtual void apply(province *province, const int change) const override
	{
		province->change_population_capacity_additive_modifier(this->population_capacity * change);
	}

	virtual void apply(holding *holding, const int change) const override
	{
		holding->change_base_population_capacity(this->population_capacity * change);
	}

private:
	int population_capacity = 0;
};

}
