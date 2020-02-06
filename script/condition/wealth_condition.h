#pragma once

#include "script/condition/condition.h"
#include "script/condition/condition_check_base.h"

namespace metternich {

template <typename T>
class wealth_condition : public condition<T>
{
public:
	wealth_condition(const int wealth, const gsml_operator effect_operator)
		: condition<T>(effect_operator), wealth(wealth)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "wealth";
		return identifier;
	}

	virtual bool check_assignment(const T *scope) const override
	{
		return this->check_greater_than_or_equality(scope);
	}

	virtual bool check_equality(const T *scope) const override
	{
		return scope->get_wealth() == this->wealth;
	}

	virtual bool check_less_than(const T *scope) const override
	{
		return scope->get_wealth() < this->wealth;
	}

	virtual bool check_greater_than(const T *scope) const override
	{
		return scope->get_wealth() > this->wealth;
	}

	virtual void bind_condition_check(condition_check_base &check, const T *scope) const override
	{
		scope->connect(scope, &T::wealth_changed, scope, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);
	}

	virtual std::string get_assignment_string() const override
	{
		return this->get_greater_than_or_equality_string();
	}

	virtual std::string get_equality_string() const override
	{
		return "Wealth is equal to " + std::to_string(this->wealth);
	}

	virtual std::string get_inequality_string() const override
	{
		return "Wealth is not equal to " + std::to_string(this->wealth);
	}

	virtual std::string get_less_than_string() const override
	{
		return "Wealth is less than " + std::to_string(this->wealth);
	}

	virtual std::string get_less_than_or_equality_string() const override
	{
		return "Wealth is less than or equal to " + std::to_string(this->wealth);
	}

	virtual std::string get_greater_than_string() const override
	{
		return "Wealth is greater than " + std::to_string(this->wealth);
	}

	virtual std::string get_greater_than_or_equality_string() const override
	{
		return "Wealth is greater than or equal to " + std::to_string(this->wealth);
	}

private:
	int wealth = 0;
};

}
