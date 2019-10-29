#pragma once

#include "script/modifier_effect/modifier_effect.h"

#include <memory>
#include <vector>

namespace metternich {

class modifier_effect;

/**
**	@brief	The modifier base class
*/
class modifier_base
{
public:
	modifier_base();
	virtual ~modifier_base();

	template <typename T>
	void apply(T *scope) const
	{
		for (const std::unique_ptr<modifier_effect> &modifier_effect : this->modifier_effects) {
			modifier_effect->apply(scope, 1);
		}
	}

	template <typename T>
	void remove(T *scope) const
	{
		for (const std::unique_ptr<modifier_effect> &modifier_effect : this->modifier_effects) {
			modifier_effect->apply(scope, -1);
		}
	}

protected:
	void add_modifier_effect(std::unique_ptr<modifier_effect> &&modifier_effect)
	{
		this->modifier_effects.push_back(std::move(modifier_effect));
	}

private:
	std::vector<std::unique_ptr<modifier_effect>> modifier_effects;
};

}
