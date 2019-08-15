#pragma once

#include "factor_modifier.h"

#include <vector>

namespace Metternich {

class GSMLData;
class GSMLProperty;

/**
**	@brief	A chance factor, used e.g. for determining the probability of a settlement holding having a particular resource; the template argument is the data type of the scope for which the chance is to be calculated
*/
template <typename T>
class ChanceFactor
{
public:
	void ProcessGSMLProperty(const GSMLProperty &property);
	void ProcessGSMLScope(const GSMLData &scope) {}

	int Calculate(T *scope) const
	{
		//get the resulting chance factor after taking into account all modifiers
		int result = this->Factor;
		return result;
	}

private:
	int Factor = 0; //the base factor for the random chance
	std::vector<FactorModifier<T>> Modifiers; //modifiers for the chance factor
};

}
