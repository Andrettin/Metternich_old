#include "script/condition/commodity_condition.h"

#include "holding/holding.h"
#include "economy/commodity.h"

namespace Metternich {

/**
**	@brief	Constructor
**
**	@param	commodity_identifier	The string identifier for the condition's commodity
*/
CommodityCondition::CommodityCondition(const std::string &commodity_identifier)
{
	this->Commodity = Commodity::Get(commodity_identifier);
}

/**
**	@brief	Check if the condition is true for a holding
**
**	@param	holding	The holding
**
**	@return	True if the condition is fulfilled, or false otherwise
*/
bool CommodityCondition::Check(const Holding *holding) const
{
	//check whether the holding's commodity is the same as that for this condition
	return holding->GetCommodity() == this->Commodity;
}

}
