#pragma once

#include "holding/building.h"
#include "holding/holding.h"
#include "script/condition/condition.h"
#include "script/condition/condition_check.h"

namespace metternich {

/**
**	@brief	A scripted has-building condition
*/
template <typename T>
class has_building_condition : public condition<T>
{
public:
	has_building_condition(const std::string &building_identifier)
	{
		this->building = building::get(building_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static std::string identifier = "has_building";
		return identifier;
	}

	virtual bool check(const T *scope) const override
	{
		return scope->has_building(this->building);
	}

	virtual void bind_condition_check(condition_check<T> &check, const T *scope) const override
	{
		scope->connect(scope, &T::buildings_changed, scope, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);
	}

private:
	building *building = nullptr;
};

}
