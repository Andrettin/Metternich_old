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

	virtual const std::string &GetIdentifier() const override
	{
		static std::string identifier = "population_capacity_modifier";
		return identifier;
	}

	virtual void Apply(Province *province, const int change) const override
	{
		province->ChangePopulationCapacityModifier(this->PopulationCapacityModifier * change);
	}

	virtual void Apply(Holding *holding, const int change) const override
	{
		holding->ChangePopulationCapacityModifier(this->PopulationCapacityModifier * change);
	}

private:
	int PopulationCapacityModifier = 0;
};

}
