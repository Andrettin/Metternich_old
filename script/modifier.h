#pragma once

#include "script/modifier_effect/modifier_effect.h"

#include <memory>
#include <vector>

namespace Metternich {

class GSMLData;
class GSMLProperty;
class ModifierEffect;

/**
**	@brief	A modifier (i.e. a collection of modifier effects)
*/
class Modifier
{
public:
	Modifier();
	~Modifier();

	void ProcessGSMLProperty(const GSMLProperty &property);
	void ProcessGSMLScope(const GSMLData &scope) { Q_UNUSED(scope); }

	template <typename T>
	void Apply(T *scope) const
	{
		for (const std::unique_ptr<ModifierEffect> &modifier_effect : this->ModifierEffects) {
			modifier_effect->Apply(scope, 1);
		}
	}

	template <typename T>
	void Remove(T *scope) const
	{
		for (const std::unique_ptr<ModifierEffect> &modifier_effect : this->ModifierEffects) {
			modifier_effect->Apply(scope, -1);
		}
	}

private:
	std::vector<std::unique_ptr<ModifierEffect>> ModifierEffects;
};

}
