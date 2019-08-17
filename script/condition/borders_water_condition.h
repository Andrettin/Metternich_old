#pragma once

#include "script/condition/condition.h"

namespace Metternich {

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

	virtual bool Check(const Province *province) const override;
	virtual bool Check(const Holding *holding) const override;

private:
	bool BordersWater = false;
};

}
