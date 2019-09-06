#pragma once

#include "script/modifier_effect/modifier_effect.h"

#include <memory>
#include <vector>

namespace metternich {

class ModifierEffect;

/**
**	@brief	The modifier base class
*/
class ModifierBase
{
public:
	ModifierBase();
	virtual ~ModifierBase();

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

protected:
	void AddModifierEffect(std::unique_ptr<ModifierEffect> &&modifier_effect)
	{
		this->ModifierEffects.push_back(std::move(modifier_effect));
	}

private:
	std::vector<std::unique_ptr<ModifierEffect>> ModifierEffects;
};

}
