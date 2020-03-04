#pragma once

#include "map/province.h"
#include "script/condition/condition.h"
#include "script/condition/condition_check_base.h"
#include "script/scope_util.h"

namespace metternich {

template <typename T>
class has_any_active_trade_route_condition final : public condition<T>
{
public:
	has_any_active_trade_route_condition(const bool has_any_active_trade_route, const gsml_operator effect_operator)
		: condition<T>(effect_operator), has_any_active_trade_route(has_any_active_trade_route)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "has_any_active_trade_route";
		return identifier;
	}

	virtual bool check_assignment(const T *scope) const override
	{
		return this->check_equality(scope);
	}

	virtual bool check_equality(const T *scope) const override
	{
		return scope->has_any_active_trade_route();
	}

	virtual void bind_condition_check(condition_check_base &check, const T *scope) const override
	{
		scope->connect(scope, &T::active_trade_routes_changed, scope, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);
	}

	virtual std::string get_assignment_string() const override
	{
		return this->get_equality_string();
	}

	virtual std::string get_equality_string() const override
	{
		if (this->has_any_active_trade_route) {
			return "Has any active trade route";
		} else {
			return "Does have any active trade route";
		}
	}

	virtual std::string get_inequality_string() const override
	{
		if (!this->has_any_active_trade_route) {
			return "Has any active trade route";
		} else {
			return "Does have any active trade route";
		}
	}

private:
	bool has_any_active_trade_route = false;
};

}
