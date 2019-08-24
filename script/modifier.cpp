#include "modifier.h"

#include "database/gsml_property.h"
#include "script/modifier_effect/modifier_effect.h"

namespace Metternich {

/**
**	@brief	Constructor
*/
Modifier::Modifier()
{
}

/**
**	@brief	Destructor
*/
Modifier::~Modifier()
{
}

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void Modifier::ProcessGSMLProperty(const GSMLProperty &property)
{
	std::unique_ptr<ModifierEffect> modifier_effect = ModifierEffect::FromGSMLProperty(property);
	this->ModifierEffects.push_back(std::move(modifier_effect));
}

}
