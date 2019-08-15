#pragma once

namespace Metternich {

class GSMLData;
class GSMLProperty;

/**
**	@brief	A modifier for a factor, e.g. a random chance, weight or mean-time-to-happen; the template argument is the data type of the scope for which the modifier's conditions are to be checked
*/
template <typename T>
class FactorModifier
{
public:
	void ProcessGSMLProperty(const GSMLProperty &property);
	void ProcessGSMLScope(const GSMLData &scope);

	int GetFactor() const
	{
		return this->Factor;
	}

	bool IsAdditive() const
	{
		return this->Additive;
	}

private:
	int Factor = 0; //the factor of the modifier itself
	bool Additive = false; //whether the modifier is additive instead of multiplicative
};

}
