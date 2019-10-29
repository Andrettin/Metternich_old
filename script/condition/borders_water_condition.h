#pragma once

#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "map/province.h"
#include "script/condition/condition.h"

namespace metternich {

/**
**	@brief	A scripted "borders water" condition
*/
class borders_water_condition : public condition
{
public:
	borders_water_condition(const bool borders_water) : borders_water(borders_water) {}

	virtual const std::string &get_identifier() const override
	{
		static std::string identifier = "borders_water";
		return identifier;
	}

	virtual bool check(const province *province) const override
	{
		return province->borders_water() == this->borders_water;
	}

	virtual bool check(const holding *holding) const override
	{
		return this->check(holding->get_province());
	}

	virtual bool check(const holding_slot *holding_slot) const override
	{
		return this->check(holding_slot->get_province());
	}

private:
	bool borders_water = false;
};

}
