#pragma once

#include "holding/holding.h"
#include "map/province.h"
#include "script/modifier_effect/modifier_effect.h"

namespace metternich {

/**
**	@brief	A population capacity modifier effect
*/
class PopulationCapacityModifierEffect : public ModifierEffect
{
public:
	PopulationCapacityModifierEffect(const int population_capacity) : PopulationCapacity(population_capacity) {}

	virtual const std::string &get_identifier() const override
	{
		static std::string identifier = "population_capacity";
		return identifier;
	}

	virtual void Apply(province *province, const int change) const override
	{
		province->change_population_capacity_additive_modifier(this->PopulationCapacity * change);
	}

	virtual void Apply(holding *holding, const int change) const override
	{
		holding->change_base_population_capacity(this->PopulationCapacity * change);
	}

private:
	int PopulationCapacity = 0;
};

}
