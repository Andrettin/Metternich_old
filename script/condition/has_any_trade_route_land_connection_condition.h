#pragma once

#include "map/province.h"
#include "script/condition/condition.h"
#include "script/scope_util.h"

namespace metternich {

template <typename T>
class has_any_trade_route_land_connection_condition : public condition<T>
{
public:
	has_any_trade_route_land_connection_condition(const bool has_any_trade_route_land_connection)
		: has_any_trade_route_land_connection(has_any_trade_route_land_connection)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "has_any_trade_route_land_connection";
		return identifier;
	}

	virtual bool check(const T *scope) const override
	{
		return scope->has_any_trade_route_land_connection();
	}

	virtual void bind_condition_check(condition_check<T> &check, const T *scope) const override
	{
		scope->connect(scope, &T::active_trade_routes_changed, scope, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);
	}

private:
	bool has_any_trade_route_land_connection = false;
};

}
