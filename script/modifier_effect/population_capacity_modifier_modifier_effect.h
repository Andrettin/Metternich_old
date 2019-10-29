#pragma once

#include "holding/holding.h"
#include "map/province.h"
#include "script/modifier_effect/modifier_effect.h"

namespace metternich {

/**
**	@brief	A population capacity modifier modifier effect
*/
class population_capacity_modifier_modifier_effect : public modifier_effect
{
public:
	population_capacity_modifier_modifier_effect(const int population_capacity_modifier) : population_capacity_modifier(population_capacity_modifier) {}

	virtual const std::string &get_identifier() const override
	{
		static std::string identifier = "population_capacity_modifier";
		return identifier;
	}

	virtual void apply(province *province, const int change) const override
	{
		province->change_population_capacity_modifier(this->population_capacity_modifier * change);
	}

	virtual void apply(holding *holding, const int change) const override
	{
		holding->change_population_capacity_modifier(this->population_capacity_modifier * change);
	}

private:
	int population_capacity_modifier = 0;
};

}
