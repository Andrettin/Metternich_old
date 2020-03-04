#pragma once

#include "map/province.h"
#include "map/terrain_type.h"
#include "script/condition/condition.h"
#include "script/condition/condition_check_base.h"
#include "script/scope_util.h"

namespace metternich {

class terrain_type;

template <typename T>
class terrain_condition final : public condition<T>
{
public:
	terrain_condition(const std::string &terrain_identifier, const gsml_operator effect_operator)
		: condition<T>(effect_operator)
	{
		this->terrain = terrain_type::get(terrain_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "terrain";
		return identifier;
	}

	virtual bool check_assignment(const T *scope) const override
	{
		return this->check_equality(scope);
	}

	virtual bool check_equality(const T *scope) const override
	{
		return scope->get_terrain() == this->terrain;
	}

	virtual void bind_condition_check(condition_check_base &check, const T *scope) const override
	{
		scope->connect(scope, &T::terrain_changed, scope, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);
	}

	virtual std::string get_assignment_string() const override
	{
		return this->get_equality_string();
	}

	virtual std::string get_equality_string() const override
	{
		return "Terrain is " + this->terrain->get_name();
	}

	virtual std::string get_inequality_string() const override
	{
		return "Terrain is not " + this->terrain->get_name();
	}

private:
	const terrain_type *terrain = nullptr;
};

}
