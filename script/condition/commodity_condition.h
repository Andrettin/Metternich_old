#pragma once

#include "economy/commodity.h"
#include "holding/holding.h"
#include "script/condition/condition.h"

namespace Metternich {

class Commodity;

/**
**	@brief	A scripted commodity condition
*/
class CommodityCondition : public Condition
{
public:
	CommodityCondition(const std::string &commodity_identifier)
	{
		this->Commodity = Commodity::Get(commodity_identifier);
	}


	virtual const std::string &GetIdentifier() const override
	{
		static std::string identifier = "commodity";
		return identifier;
	}

	virtual bool Check(const Holding *holding) const override
	{
		//check whether the holding's commodity is the same as that for this condition
		return holding->GetCommodity() == this->Commodity;
	}

private:
	Metternich::Commodity *Commodity = nullptr;
};

}