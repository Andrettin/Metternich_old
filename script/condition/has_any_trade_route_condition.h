#pragma once

#include "map/province.h"
#include "script/condition/condition.h"
#include "script/scope_util.h"

namespace metternich {

template <typename T>
class has_any_trade_route_condition : public condition<T>
{
public:
	has_any_trade_route_condition(const bool has_any_trade_route) : has_any_trade_route(has_any_trade_route)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "has_any_trade_route";
		return identifier;
	}

	virtual bool check(const T *scope) const override
	{
		const province *province = get_scope_province(scope);
		return province->has_any_trade_route() == this->has_any_trade_route;
	}

private:
	bool has_any_trade_route = false;
};

}
