#include "script/condition/terrain_condition.h"

#include "holding/holding.h"
#include "map/province.h"
#include "map/terrain.h"

namespace Metternich {

/**
**	@brief	Constructor
**
**	@param	terrain_identifier	The string identifier for the condition's terrain
*/
TerrainCondition::TerrainCondition(const std::string &terrain_identifier)
{
	this->Terrain = Terrain::Get(terrain_identifier);
}

/**
**	@brief	Check if the condition is true for a province
**
**	@param	province	The province
**
**	@return	True if the condition is fulfilled, or false otherwise
*/
bool TerrainCondition::Check(const Province *province) const
{
	return province->GetTerrain() == this->Terrain;
}

/**
**	@brief	Check if the condition is true for a holding
**
**	@param	holding	The holding
**
**	@return	True if the condition is fulfilled, or false otherwise
*/
bool TerrainCondition::Check(const Holding *holding) const
{
	return this->Check(holding->GetProvince());
}

}
