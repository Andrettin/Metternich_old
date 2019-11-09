#pragma once

#include "economy/commodity.h"
#include "holding/holding.h"
#include "script/condition/condition.h"
#include "script/condition/condition_check.h"

namespace metternich {

class commodity;

/**
**	@brief	A scripted commodity condition
*/
class commodity_condition : public condition
{
public:
	commodity_condition(const std::string &commodity_identifier)
	{
		this->commodity = commodity::get(commodity_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static std::string identifier = "commodity";
		return identifier;
	}

	virtual bool check(const holding *holding) const override
	{
		//check whether the holding's commodity is the same as that for this condition
		return holding->get_commodity() == this->commodity;
	}

	virtual void bind_condition_check(condition_check<holding> &check, const holding *holding) const override
	{
		holding->connect(holding, &holding::commodity_changed, holding, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);
	}

private:
	metternich::commodity *commodity = nullptr;
};

}
