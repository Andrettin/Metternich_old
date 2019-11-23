#pragma once

#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "map/province.h"
#include "map/world.h"
#include "script/condition/condition.h"

namespace metternich {

class world;

/**
**	@brief	A scripted world condition
*/
class world_condition : public condition
{
public:
	world_condition(const std::string &world_identifier)
	{
		this->world = world::get(world_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static std::string identifier = "world";
		return identifier;
	}

	virtual bool check(const province *province) const override
	{
		return province->get_world() == this->world;
	}

	virtual bool check(const holding *holding) const override
	{
		return this->check(holding->get_province());
	}

	virtual bool check(const holding_slot *holding_slot) const override
	{
		return this->check(holding_slot->get_province());
	}

private:
	const world *world = nullptr;
};

}
