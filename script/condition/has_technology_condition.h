#pragma once

#include "map/province.h"
#include "script/condition/condition.h"
#include "script/condition/condition_check.h"
#include "technology/technology.h"

namespace metternich {

template <typename T>
class has_technology_condition : public condition<T>
{
public:
	has_technology_condition(const std::string &technology_identifier)
	{
		this->technology = technology::get(technology_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "has_technology";
		return identifier;
	}

	virtual bool check(const T *scope) const override
	{
		const province *province = get_scope_province(scope);
		return province->has_technology(this->technology);
	}

	virtual void bind_condition_check(condition_check<T> &check, const T *scope) const override
	{
		const province *province = get_scope_province(scope);
		scope->connect(province, &province::technologies_changed, scope, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);
	}

private:
	technology *technology = nullptr;
};

}
