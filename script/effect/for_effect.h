#pragma once

#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "script/effect/effect.h"
#include "script/effect/effect_list.h"

namespace metternich {

template <typename T>
class for_effect : public effect<T>
{
public:
	for_effect(const gsml_operator effect_operator) : effect<T>(effect_operator)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "for";
		return identifier;
	}

	virtual void process_gsml_property(const gsml_property &property) override
	{
		if (property.get_key() == "count") {
			this->count = std::stoi(property.get_value());
		} else {
			this->effects.process_gsml_property(property);
		}
	}

	virtual void process_gsml_scope(const gsml_data &scope) override
	{
		this->effects.process_gsml_scope(scope);
	}

	virtual void do_assignment_effect(T *scope) const override
	{
		for (int i = 0; i < this->count; ++i) {
			this->effects.do_effects(scope);
		}
	}

	virtual std::string get_assignment_string(const T *scope, const size_t indent) const override
	{
		std::string str = "This will occur " + std::to_string(this->count) + " times:\n";
		str += this->effects.get_effects_string(scope, indent + 1);
		return str;
	}

private:
	int count = 0;
	effect_list<T> effects;
};

}
