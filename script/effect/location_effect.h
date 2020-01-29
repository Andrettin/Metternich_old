#pragma once

#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "script/effect/effect.h"
#include "script/effect/effect_list.h"

namespace metternich {

class province;

template <typename T>
class location_effect : public effect<T>
{
public:
	location_effect(const gsml_operator effect_operator) : effect<T>(effect_operator)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "location";
		return identifier;
	}

	virtual void process_gsml_property(const gsml_property &property) override
	{
		this->effects.process_gsml_property(property);
	}

	virtual void process_gsml_scope(const gsml_data &scope) override
	{
		this->effects.process_gsml_scope(scope);
	}

	virtual void do_assignment_effect(T *scope) const override
	{
		this->effects.do_effects(scope->get_location());
	}

	virtual std::string get_assignment_string(const T *scope, const size_t indent) const override
	{
		std::string effects_string = this->effects.get_effects_string(scope->get_location(), indent + 1);

		if (effects_string.empty()) {
			return std::string();
		}

		std::string str = scope->get_location()->get_name() + ":\n" + effects_string;

		return str;
	}

private:
	effect_list<province> effects;
};

}
