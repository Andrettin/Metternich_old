#pragma once

#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "script/effect/effect.h"
#include "script/effect/scripted_effect.h"

namespace metternich {

template <typename T>
class scripted_effect_effect : public effect<T>
{
public:
	scripted_effect_effect(const std::string &effect_identifier, const gsml_operator effect_operator)
		: effect<T>(effect_operator)
	{
		if constexpr (std::is_same_v<T, character>) {
			this->scripted_effect = character_scripted_effect::get(effect_identifier);
		}
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "scripted_effect";
		return identifier;
	}

	virtual void do_assignment_effect(T *scope) const override
	{
		this->scripted_effect->do_effects(scope);
	}

	virtual std::string get_assignment_string(const T *scope, const size_t indent) const override
	{
		return this->scripted_effect->get_effects_string(scope, indent);
	}

private:
	const scripted_effect<T> *scripted_effect = nullptr;
};

}
