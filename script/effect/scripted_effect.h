#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"
#include "script/effect/effect_list.h"

#include <string>

namespace metternich {

class character;

//the class for a predefined, reusable scripted effect
template <typename T>
class scripted_effect
{
public:
	void process_gsml_property(const gsml_property &property)
	{
		this->effects.process_gsml_property(property);
	}

	void process_gsml_scope(const gsml_data &scope)
	{
		this->effects.process_gsml_scope(scope);
	}

	void do_effects(T *scope) const
	{
		this->effects.do_effects(scope);
	}

	std::string get_effects_string(const T *scope, const size_t indent) const
	{
		return this->effects.get_effects_string(scope, indent);
	}

private:
	effect_list<T> effects;
};

class character_scripted_effect : public data_entry, public data_type<character_scripted_effect>, public scripted_effect<character>
{
	Q_OBJECT

public:
	static constexpr const char *class_identifier = "character_scripted_effect";
	static constexpr const char *database_folder = "character_scripted_effects";

public:
	character_scripted_effect(const std::string &identifier) : data_entry(identifier)
	{
	}

	virtual void process_gsml_property(const gsml_property &property) override
	{
		scripted_effect::process_gsml_property(property);
	}

	virtual void process_gsml_scope(const gsml_data &scope) override
	{
		scripted_effect::process_gsml_scope(scope);
	}
};

}
