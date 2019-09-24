#pragma once

#include "holding/holding.h"
#include "map/province.h"
#include "script/modifier_effect/modifier_effect.h"

namespace metternich {

/**
**	@brief	A population growth modifier effect
*/
class PopulationGrowthModifierEffect : public ModifierEffect
{
public:
	PopulationGrowthModifierEffect(const int population_growth) : PopulationGrowth(population_growth) {}

	virtual const std::string &get_identifier() const override
	{
		static std::string identifier = "population_growth";
		return identifier;
	}

	virtual void Apply(Province *province, const int change) const override
	{
		province->ChangePopulationGrowthModifier(this->PopulationGrowth * change);
	}

	virtual void Apply(holding *holding, const int change) const override
	{
		holding->change_base_population_growth(this->PopulationGrowth * change);
	}

private:
	int PopulationGrowth = 0;
};

}
