#pragma once

#include "holding/holding.h"
#include "map/province.h"
#include "map/terrain.h"
#include "script/condition/condition.h"

namespace metternich {

class Terrain;

/**
**	@brief	A scripted terrain condition
*/
class TerrainCondition : public Condition
{
public:
	TerrainCondition(const std::string &terrain_identifier)
	{
		this->Terrain = Terrain::Get(terrain_identifier);
	}

	virtual const std::string &GetIdentifier() const override
	{
		static std::string identifier = "terrain";
		return identifier;
	}

	virtual bool Check(const Province *province) const override
	{
		return province->GetTerrain() == this->Terrain;
	}

	virtual bool Check(const holding *holding) const override
	{
		return this->Check(holding->get_province());
	}

private:
	metternich::Terrain *Terrain = nullptr;
};

}
