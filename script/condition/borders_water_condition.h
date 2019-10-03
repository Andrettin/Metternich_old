#pragma once

#include "holding/holding.h"
#include "map/province.h"
#include "script/condition/condition.h"

namespace metternich {

/**
**	@brief	A scripted "borders water" condition
*/
class BordersWaterCondition : public Condition
{
public:
	BordersWaterCondition(const bool borders_water) : BordersWater(borders_water) {}

	virtual const std::string &get_identifier() const override
	{
		static std::string identifier = "borders_water";
		return identifier;
	}

	virtual bool check(const province *province) const override
	{
		return province->borders_water() == this->BordersWater;
	}

	virtual bool check(const holding *holding) const override
	{
		return this->check(holding->get_province());
	}

private:
	bool BordersWater = false;
};

}
