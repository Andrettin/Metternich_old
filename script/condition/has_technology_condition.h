#pragma once

#include "script/condition/condition.h"

namespace metternich {

class technology;

template <typename T>
class has_technology_condition : public condition<T>
{
public:
	has_technology_condition(const std::string &technology_identifier);

	has_technology_condition(technology *technology) : technology(technology)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "has_technology";
		return identifier;
	}

	virtual bool check(const T *scope) const override;
	virtual void bind_condition_check(condition_check_base &check, const T *scope) const override;

private:
	technology *technology = nullptr;
};

extern template class has_technology_condition<holding>;
extern template class has_technology_condition<holding_slot>;
extern template class has_technology_condition<population_unit>;
extern template class has_technology_condition<province>;

}
