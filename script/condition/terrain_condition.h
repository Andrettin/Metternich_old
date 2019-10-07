#pragma once

#include "holding/holding.h"
#include "map/province.h"
#include "map/terrain_type.h"
#include "script/condition/condition.h"

namespace metternich {

class terrain_type;

/**
**	@brief	A scripted terrain condition
*/
class terrain_condition : public Condition
{
public:
	terrain_condition(const std::string &terrain_identifier)
	{
		this->terrain = terrain_type::get(terrain_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static std::string identifier = "terrain";
		return identifier;
	}

	virtual bool check(const province *province) const override
	{
		return province->get_terrain() == this->terrain;
	}

	virtual bool check(const holding *holding) const override
	{
		return this->check(holding->get_province());
	}

private:
	terrain_type *terrain = nullptr;
};

}
