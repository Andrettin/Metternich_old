#pragma once

#include "holding/holding.h"
#include "holding/holding_type.h"
#include "script/condition/condition.h"
#include "script/condition/condition_check_base.h"

namespace metternich {

template <typename T>
class holding_type_condition final : public condition<T>
{
public:
	holding_type_condition(const std::string &holding_type_identifier, const gsml_operator effect_operator)
		: condition<T>(effect_operator)
	{
		this->holding_type = holding_type::get(holding_type_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "holding_type";
		return identifier;
	}

	virtual bool check_assignment(const T *scope) const override
	{
		return this->check_equality(scope);
	}

	virtual bool check_equality(const T *scope) const override
	{
		return scope->get_type() == this->holding_type;
	}

	virtual void bind_condition_check(condition_check_base &check, const T *scope) const override
	{
		scope->connect(scope, &T::type_changed, scope, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);
	}

	virtual std::string get_assignment_string() const override
	{
		return this->get_equality_string();
	}

	virtual std::string get_equality_string() const override
	{
		return "Holding type is " + this->holding_type->get_name();
	}

	virtual std::string get_inequality_string() const override
	{
		return "Holding type is not " + this->holding_type->get_name();
	}

private:
	const holding_type *holding_type = nullptr;
};

}
