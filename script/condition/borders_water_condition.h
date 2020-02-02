#pragma once

#include "map/province.h"
#include "script/condition/condition.h"
#include "script/scope_util.h"

namespace metternich {

template <typename T>
class borders_water_condition : public condition<T>
{
public:
	borders_water_condition(const bool borders_water, const gsml_operator effect_operator)
		: condition<T>(effect_operator), borders_water(borders_water)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "borders_water";
		return identifier;
	}

	virtual bool check_assignment(const T *scope) const override
	{
		return this->check_equality(scope);
	}

	virtual bool check_equality(const T *scope) const override
	{
		const province *province = get_scope_province(scope);
		return province->borders_water() == this->borders_water;
	}

private:
	bool borders_water = false;
};

}
