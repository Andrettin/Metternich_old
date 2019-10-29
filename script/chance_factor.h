#pragma once

#include "factor_modifier.h"

#include <memory>
#include <vector>

namespace metternich {

class factor_modifier;
class gsml_data;
class gsml_property;

/**
**	@brief	A chance factor, used e.g. for determining the probability of a settlement holding having a particular resource
*/
class chance_factor
{
public:
	chance_factor();
	~chance_factor();

	void process_gsml_property(const gsml_property &property);
	void process_gsml_scope(const gsml_data &scope);

	template <typename T>
	int calculate(const T *scope) const
	{
		//get the resulting chance factor after taking into account all modifiers
		int result = this->factor;

		for (const std::unique_ptr<factor_modifier> &modifier : this->modifiers) {
			if (modifier->check_conditions(scope)) {
				result *= modifier->get_factor();
				result /= 100;
			}
		}

		return result;
	}

private:
	int factor = 0; //the base factor for the random chance
	std::vector<std::unique_ptr<factor_modifier>> modifiers; //modifiers for the chance factor
};

}
