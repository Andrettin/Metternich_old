#include "script/identifiable_modifier.h"

#include "database/gsml_property.h"
#include "script/modifier_effect/modifier_effect.h"

namespace Metternich {

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void IdentifiableModifier::ProcessGSMLProperty(const GSMLProperty &property)
{
	std::unique_ptr<ModifierEffect> modifier_effect = ModifierEffect::FromGSMLProperty(property);
	this->AddModifierEffect(std::move(modifier_effect));
}

}
