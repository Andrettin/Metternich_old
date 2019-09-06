#pragma once

#include "script/condition/condition.h"

#include <memory>
#include <vector>

namespace metternich {

class Condition;
class gsml_data;
class GSMLProperty;

/**
**	@brief	A modifier for a factor, e.g. a random chance, weight or mean-time-to-happen
*/
class FactorModifier
{
public:
	FactorModifier();
	~FactorModifier();

	void ProcessGSMLProperty(const GSMLProperty &property);
	void ProcessGSMLScope(const gsml_data &scope);

	int GetFactor() const
	{
		return this->Factor;
	}

	bool IsAdditive() const
	{
		return this->Additive;
	}

	template <typename T>
	bool CheckConditions(T *scope) const
	{
		for (const std::unique_ptr<Condition> &condition : this->Conditions) {
			if (!condition->Check(scope)) {
				return false;
			}
		}
		return true;
	}

private:
	int Factor = 0; //the factor of the modifier itself
	bool Additive = false; //whether the modifier is additive instead of multiplicative
	std::vector<std::unique_ptr<Condition>> Conditions; //conditions for whether the modifier is to be applied
};

}
