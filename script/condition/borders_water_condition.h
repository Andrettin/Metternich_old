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

	virtual const std::string &GetIdentifier() const override
	{
		static std::string identifier = "borders_water";
		return identifier;
	}

	virtual bool Check(const Province *province) const override
	{
		return province->BordersWater() == this->BordersWater;
	}

	virtual bool Check(const Holding *holding) const override
	{
		return this->Check(holding->GetProvince());
	}

private:
	bool BordersWater = false;
};

}
