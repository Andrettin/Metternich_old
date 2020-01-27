#pragma once

#include "map/province.h"
#include "map/terrain_type.h"
#include "script/condition/condition.h"
#include "script/scope_util.h"

namespace metternich {

class terrain_type;

template <typename T>
class terrain_condition : public condition<T>
{
public:
	terrain_condition(const std::string &terrain_identifier)
	{
		this->terrain = terrain_type::get(terrain_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "terrain";
		return identifier;
	}

	virtual bool check(const T *scope) const override
	{
		const province *province = get_scope_province(scope);
		return province->get_terrain() == this->terrain;
	}

private:
	const terrain_type *terrain = nullptr;
};

}
