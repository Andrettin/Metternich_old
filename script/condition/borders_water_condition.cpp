#include "script/condition/borders_water_condition.h"

#include "holding/holding.h"
#include "map/province.h"
#include "map/terrain.h"

namespace Metternich {

/**
**	@brief	Check if the condition is true for a province
**
**	@param	province	The province
**
**	@return	True if the condition is fulfilled, or false otherwise
*/
bool BordersWaterCondition::Check(const Province *province) const
{
	return province->BordersWater() == this->BordersWater;
}

/**
**	@brief	Check if the condition is true for a holding
**
**	@param	holding	The holding
**
**	@return	True if the condition is fulfilled, or false otherwise
*/
bool BordersWaterCondition::Check(const Holding *holding) const
{
	return this->Check(holding->GetProvince());
}

}
