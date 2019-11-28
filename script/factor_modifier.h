#pragma once

#include "script/condition/condition.h"

#include <memory>
#include <vector>

namespace metternich {

class gsml_data;
class gsml_property;

template <typename T>
class condition;

/**
**	@brief	A modifier for a factor, e.g. a random chance, weight or mean-time-to-happen
*/
template <typename T>
class factor_modifier
{
public:
	factor_modifier();
	~factor_modifier();

	void process_gsml_property(const gsml_property &property);
	void process_gsml_scope(const gsml_data &scope);

	int get_factor() const
	{
		return this->factor;
	}

	bool is_additive() const
	{
		return this->additive;
	}

	bool check_conditions(const T *scope) const
	{
		for (const std::unique_ptr<condition<T>> &condition : this->conditions) {
			if (!condition->check(scope)) {
				return false;
			}
		}
		return true;
	}

private:
	int factor = 0; //the factor of the modifier itself
	bool additive = false; //whether the modifier is additive instead of multiplicative
	std::vector<std::unique_ptr<condition<T>>> conditions; //conditions for whether the modifier is to be applied
};

extern template class factor_modifier<holding_slot>;

}
