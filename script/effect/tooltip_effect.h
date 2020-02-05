#pragma once

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "script/effect/effect.h"
#include "script/effect/effect_list.h"
#include "translator.h"

namespace metternich {

//a scripted effect which displays a custom tooltip for the effects it encloses
template <typename T>
class tooltip_effect : public effect<T>
{
public:
	tooltip_effect(const gsml_operator effect_operator) : effect<T>(effect_operator)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "tooltip";
		return identifier;
	}

	virtual void process_gsml_property(const gsml_property &property) override
	{
		if (property.get_key() == "text") {
			this->text = property.get_value();
		} else {
			this->effects.process_gsml_property(property);
		}
	}

	virtual void process_gsml_scope(const gsml_data &scope) override
	{
		this->effects.process_gsml_scope(scope);
	}

	virtual void do_assignment_effect(T *scope, const context &ctx) const override
	{
		this->effects.do_effects(scope, ctx);
	}

	virtual std::string get_assignment_string() const override
	{
		return translator::get()->translate(this->text);
	}

private:
	std::string text;
	effect_list<T> effects;
};

}
