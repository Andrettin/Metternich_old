#pragma once

#include "holding/holding.h"
#include "map/province.h"
#include "script/modifier_effect/modifier_effect.h"

namespace Metternich {

/**
**	@brief	A population capacity modifier effect
*/
class PopulationCapacityModifierEffect : public ModifierEffect
{
public:
	PopulationCapacityModifierEffect(const int population_capacity) : PopulationCapacity(population_capacity) {}

	virtual const std::string &GetIdentifier() const override
	{
		static std::string identifier = "population_capacity";
		return identifier;
	}

	virtual void Apply(Province *province, const int change) const override
	{
		province->ChangePopulationCapacityAdditiveModifier(this->PopulationCapacity * change);
	}

	virtual void Apply(Holding *holding, const int change) const override
	{
		holding->ChangeBasePopulationCapacity(this->PopulationCapacity * change);
	}

private:
	int PopulationCapacity = 0;
};

}
