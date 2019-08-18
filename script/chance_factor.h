#pragma once

#include "factor_modifier.h"

#include <memory>
#include <vector>

namespace Metternich {

class FactorModifier;
class GSMLData;
class GSMLProperty;

/**
**	@brief	A chance factor, used e.g. for determining the probability of a settlement holding having a particular resource
*/
class ChanceFactor
{
public:
	ChanceFactor();
	~ChanceFactor();

	void ProcessGSMLProperty(const GSMLProperty &property);
	void ProcessGSMLScope(const GSMLData &scope);

	template <typename T>
	int Calculate(const T *scope) const
	{
		//get the resulting chance factor after taking into account all modifiers
		int result = this->Factor;

		for (const std::unique_ptr<FactorModifier> &modifier : this->Modifiers) {
			if (modifier->CheckConditions(scope)) {
				result *= modifier->GetFactor();
				result /= 100;
			}
		}

		return result;
	}

private:
	int Factor = 0; //the base factor for the random chance
	std::vector<std::unique_ptr<FactorModifier>> Modifiers; //modifiers for the chance factor
};

}