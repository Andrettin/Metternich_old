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
template <typename T>
class world_condition : public condition<T>
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

	virtual bool check(const T *scope) const override
	{
		const province *province = nullptr;
		if constexpr (std::is_same_v<T, metternich::province>) {
			province = scope;
		} else {
			province = scope->get_province();
		}
		return province->get_world() == this->world;
	}

private:
	const world *world = nullptr;
};

}
