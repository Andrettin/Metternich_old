#pragma once

#include "script/condition/condition.h"

#include <memory>
#include <vector>

namespace metternich {

class condition;
class gsml_data;
class gsml_property;

/**
**	@brief	A modifier for a factor, e.g. a random chance, weight or mean-time-to-happen
*/
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

	template <typename T>
	bool check_conditions(T *scope) const
	{
		for (const std::unique_ptr<condition> &condition : this->conditions) {
			if (!condition->check(scope)) {
				return false;
			}
		}
		return true;
	}

private:
	int factor = 0; //the factor of the modifier itself
	bool additive = false; //whether the modifier is additive instead of multiplicative
	std::vector<std::unique_ptr<condition>> conditions; //conditions for whether the modifier is to be applied
};

}
