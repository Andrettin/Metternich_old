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
		this->Terrain = Terrain::get(terrain_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static std::string identifier = "terrain";
		return identifier;
	}

	virtual bool check(const Province *province) const override
	{
		return province->GetTerrain() == this->Terrain;
	}

	virtual bool check(const holding *holding) const override
	{
		return this->check(holding->get_province());
	}

private:
	metternich::Terrain *Terrain = nullptr;
};

}
