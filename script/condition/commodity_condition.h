#pragma once

#include "script/condition/condition.h"

namespace Metternich {

class Commodity;

/**
**	@brief	A scripted commodity condition
*/
class CommodityCondition : public Condition
{
public:
	CommodityCondition(const std::string &commodity_identifier);

	virtual const std::string &GetIdentifier() const override
	{
		static std::string identifier = "commodity";
		return identifier;
	}

	virtual bool Check(const Holding *holding) const override;

private:
	Metternich::Commodity *Commodity = nullptr;
};

}
