#pragma once

#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "map/province.h"
#include "map/world.h"
#include "script/condition/condition.h"
#include "script/scope_util.h"

namespace metternich {

class world;

template <typename T>
class world_condition : public condition<T>
{
public:
	world_condition(const std::string &world_identifier, const gsml_operator effect_operator)
		: condition<T>(effect_operator)
	{
		this->world = world::get(world_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "world";
		return identifier;
	}

	virtual bool check_assignment(const T *scope) const override
	{
		return this->check_equality(scope);
	}

	virtual bool check_equality(const T *scope) const override
	{
		const province *province = get_scope_province(scope);
		return province->get_world() == this->world;
	}

private:
	const world *world = nullptr;
};

}
