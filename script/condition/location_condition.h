#pragma once

#include "map/province.h"
#include "script/condition/condition_check_base.h"
#include "script/condition/scope_condition.h"
#include "util/string_util.h"

namespace metternich {

class province;

template <typename T>
class location_condition : public scope_condition<T, province>
{
public:
	location_condition(const gsml_operator effect_operator) : scope_condition<T, province>(effect_operator)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "location";
		return identifier;
	}

	virtual province *get_scope(const T *scope) const override
	{
		return scope->get_location();
	}

	virtual void bind_condition_check(condition_check_base &check, const T *scope) const override
	{
		scope->connect(scope, &T::location_changed, scope, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);

		scope_condition<T, province>::bind_condition_check(check, scope);
	}
};

}
