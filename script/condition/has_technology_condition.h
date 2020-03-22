#pragma once

#include "database/gsml_operator.h"
#include "script/condition/condition.h"

namespace metternich {

class technology;

template <typename T>
class has_technology_condition final : public condition<T>
{
public:
	has_technology_condition(const std::string &technology_identifier, const gsml_operator effect_operator);

	has_technology_condition(technology *technology, const gsml_operator effect_operator = gsml_operator::assignment) : condition<T>(effect_operator), technology(technology)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "has_technology";
		return identifier;
	}

	virtual bool check_assignment(const T *scope) const override;
	virtual void bind_condition_check(condition_check_base &check, const T *scope) const override;

	virtual std::string get_assignment_string() const override
	{
		return this->get_equality_string();
	}

	virtual std::string get_equality_string() const override;
	virtual std::string get_inequality_string() const override;

private:
	technology *technology = nullptr;
};

extern template class has_technology_condition<holding>;
extern template class has_technology_condition<holding_slot>;
extern template class has_technology_condition<population_unit>;
extern template class has_technology_condition<province>;
extern template class has_technology_condition<territory>;

}
