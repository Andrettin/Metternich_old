#pragma once

#include <memory>
#include <vector>

namespace metternich {

class character;
class gsml_data;
class gsml_property;
class holding_slot;

template <typename T>
class factor_modifier;

/**
**	@brief	A chance factor, used e.g. for determining the probability of a settlement holding having a particular resource
*/
template <typename T>
class chance_factor
{
public:
	chance_factor();
	chance_factor(const int factor);
	~chance_factor();

	void process_gsml_property(const gsml_property &property);
	void process_gsml_scope(const gsml_data &scope);

	int calculate(const T *scope) const;

private:
	int factor = 0; //the base factor for the random chance
	std::vector<std::unique_ptr<factor_modifier<T>>> modifiers; //modifiers for the chance factor
};

extern template class chance_factor<character>;
extern template class chance_factor<holding_slot>;

}
