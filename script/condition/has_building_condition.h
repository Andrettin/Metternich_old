#pragma once

#include "holding/building.h"
#include "holding/holding.h"
#include "script/condition/condition.h"
#include "script/condition/condition_check_base.h"

namespace metternich {

template <typename T>
class has_building_condition final : public condition<T>
{
public:
	has_building_condition(const std::string &building_identifier, const gsml_operator effect_operator)
		: condition<T>(effect_operator)
	{
		this->building = building::get(building_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "has_building";
		return identifier;
	}

	virtual bool check_assignment(const T *scope) const override
	{
		return scope->has_building(this->building);
	}

	virtual void bind_condition_check(condition_check_base &check, const T *scope) const override
	{
		scope->connect(scope, &T::buildings_changed, scope, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);
	}

	virtual std::string get_assignment_string() const override
	{
		return this->get_equality_string();
	}

	virtual std::string get_equality_string() const override
	{
		return "Has the " + this->building->get_name() + " building";
	}

	virtual std::string get_inequality_string() const override
	{
		return "Does not have the " + this->building->get_name() + " building";
	}

private:
	building *building = nullptr;
};

}
