#pragma once

#include "holding/holding.h"
#include "map/province.h"
#include "script/modifier_effect/modifier_effect.h"

namespace metternich {

/**
**	@brief	A population capacity modifier modifier effect
*/
class PopulationCapacityModifierModifierEffect : public ModifierEffect
{
public:
	PopulationCapacityModifierModifierEffect(const int population_capacity_modifier) : PopulationCapacityModifier(population_capacity_modifier) {}

	virtual const std::string &get_identifier() const override
	{
		static std::string identifier = "population_capacity_modifier";
		return identifier;
	}

	virtual void Apply(province *province, const int change) const override
	{
		province->change_population_capacity_modifier(this->PopulationCapacityModifier * change);
	}

	virtual void Apply(holding *holding, const int change) const override
	{
		holding->change_population_capacity_modifier(this->PopulationCapacityModifier * change);
	}

private:
	int PopulationCapacityModifier = 0;
};

}
