#pragma once

#include "map/province.h"
#include "script/condition/condition.h"
#include "script/scope_util.h"

namespace metternich {

template <typename T>
class has_any_trade_route_condition : public condition<T>
{
public:
	has_any_trade_route_condition(const bool has_any_trade_route, const gsml_operator effect_operator)
		: condition<T>(effect_operator), has_any_trade_route(has_any_trade_route)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "has_any_trade_route";
		return identifier;
	}

	virtual bool check_assignment(const T *scope) const override
	{
		return this->check_equality(scope);
	}

	virtual bool check_equality(const T *scope) const override
	{
		return scope->has_any_trade_route() == this->has_any_trade_route;
	}

private:
	bool has_any_trade_route = false;
};

}
