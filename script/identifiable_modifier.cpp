#include "script/identifiable_modifier.h"

#include "database/gsml_property.h"
#include "script/modifier_effect/modifier_effect.h"

namespace metternich {

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void IdentifiableModifier::ProcessGSMLProperty(const gsml_property &property)
{
	std::unique_ptr<ModifierEffect> modifier_effect = ModifierEffect::FromGSMLProperty(property);
	this->AddModifierEffect(std::move(modifier_effect));
}

}
