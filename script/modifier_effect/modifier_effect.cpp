#include "script/modifier_effect/modifier_effect.h"

#include "database/gsml_property.h"
#include "script/modifier_effect/population_capacity_modifier_effect.h"
#include "script/modifier_effect/population_capacity_modifier_modifier_effect.h"
#include "script/modifier_effect/population_growth_modifier_effect.h"
#include "util.h"

namespace metternich {

/**
**	@brief	Create a modifier effect from a GSML property
**
**	@param	gsml_property	The GSML property
*/
std::unique_ptr<ModifierEffect> ModifierEffect::FromGSMLProperty(const gsml_property &property)
{
	std::string identifier = property.get_key();
	std::unique_ptr<ModifierEffect> modifier_effect;

	if (identifier == "population_capacity") {
		modifier_effect = std::make_unique<PopulationCapacityModifierEffect>(std::stoi(property.get_value()));
	} else if (identifier == "population_capacity_modifier") {
		modifier_effect = std::make_unique<PopulationCapacityModifierModifierEffect>(CentesimalNumberStringToInt(property.get_value()));
	} else if (identifier == "population_growth") {
		modifier_effect = std::make_unique<PopulationGrowthModifierEffect>(FractionalNumberStringToInt<4>(property.get_value()));
	} else {
		throw std::runtime_error("Invalid modifier effect: \"" + identifier + "\".");
	}

	return modifier_effect;
}

}
