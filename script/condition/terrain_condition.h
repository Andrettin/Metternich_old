#pragma once

#include "script/condition/condition.h"

namespace Metternich {

class Terrain;

/**
**	@brief	A scripted terrain condition
*/
class TerrainCondition : public Condition
{
public:
	TerrainCondition(const std::string &terrain_identifier);

	virtual const std::string &GetIdentifier() const override
	{
		static std::string identifier = "terrain";
		return identifier;
	}

	virtual bool Check(const Holding *holding) const override;

private:
	Metternich::Terrain *Terrain = nullptr;;
};

}
