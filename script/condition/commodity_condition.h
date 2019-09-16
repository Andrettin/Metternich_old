#pragma once

#include "economy/commodity.h"
#include "holding/holding.h"
#include "script/condition/condition.h"

namespace metternich {

class commodity;

/**
**	@brief	A scripted commodity condition
*/
class commodity_condition : public Condition
{
public:
	commodity_condition(const std::string &commodity_identifier)
	{
		this->commodity = commodity::Get(commodity_identifier);
	}


	virtual const std::string &GetIdentifier() const override
	{
		static std::string identifier = "commodity";
		return identifier;
	}

	virtual bool Check(const holding *holding) const override
	{
		//check whether the holding's commodity is the same as that for this condition
		return holding->get_commodity() == this->commodity;
	}

private:
	metternich::commodity *commodity = nullptr;
};

}
