#pragma once

#include "economy/commodity.h"
#include "script/condition/condition.h"
#include "script/condition/condition_check_base.h"

namespace metternich {

class commodity;

template <typename T>
class commodity_condition : public condition<T>
{
public:
	commodity_condition(const std::string &commodity_identifier, const gsml_operator effect_operator)
		: condition<T>(effect_operator)
	{
		this->commodity = commodity::get(commodity_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "commodity";
		return identifier;
	}

	virtual bool check_assignment(const T *scope) const override
	{
		return this->check_equality(scope);
	}

	virtual bool check_equality(const T *scope) const override
	{
		//check whether the scope's commodity is the same as that for this condition
		return scope->get_commodity() == this->commodity;
	}

	virtual void bind_condition_check(condition_check_base &check, const T *scope) const override
	{
		scope->connect(scope, &T::commodity_changed, scope, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);
	}

	virtual std::string get_assignment_string() const override
	{
		return this->get_equality_string();
	}

	virtual std::string get_equality_string() const override
	{
		return "Commodity is " + this->commodity->get_name();
	}

	virtual std::string get_inequality_string() const override
	{
		return "Commodity is not " + this->commodity->get_name();
	}

private:
	const metternich::commodity *commodity = nullptr;
};

}
