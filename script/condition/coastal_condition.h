#pragma once

#include "map/province.h"
#include "script/condition/condition.h"
#include "script/scope_util.h"

namespace metternich {

template <typename T>
class coastal_condition final : public condition<T>
{
public:
	coastal_condition(const bool coastal, const gsml_operator effect_operator)
		: condition<T>(effect_operator), coastal(coastal)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "coastal";
		return identifier;
	}

	virtual bool check_assignment(const T *scope) const override
	{
		return this->check_equality(scope);
	}

	virtual bool check_equality(const T *scope) const override
	{
		const province *province = get_scope_province(scope);
		return province != nullptr && province->is_coastal() == this->coastal;
	}

	virtual std::string get_assignment_string() const override
	{
		return this->get_equality_string();
	}

	virtual std::string get_equality_string() const override
	{
		if (this->coastal) {
			return "Coastal";
		} else {
			return "Not coastal";
		}
	}

	virtual std::string get_inequality_string() const override
	{
		if (!this->coastal) {
			return "Coastal";
		} else {
			return "Not coastal";
		}
	}

private:
	bool coastal = false;
};

}
