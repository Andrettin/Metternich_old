#pragma once

#include "character/trait.h"
#include "script/effect/effect.h"
#include "util/string_util.h"

#include <type_traits>

namespace metternich {

class trait;

template <typename T>
class traits_effect : public effect<T>
{
public:
	traits_effect(const std::string &trait_identifier, const gsml_operator effect_operator)
		: effect<T>(effect_operator)
	{
		this->trait = trait::get(trait_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "traits";
		return identifier;
	}

	virtual void do_addition_effect(T *scope) const override
	{
		scope->add_trait(this->trait);
	}

	virtual void do_subtraction_effect(T *scope) const override
	{
		scope->remove_trait(this->trait);
	}

	virtual std::string get_addition_string() const override
	{
		return "Add " + string::highlight(this->trait->get_name()) + " Trait";
	}

	virtual std::string get_subtraction_string() const override
	{
		return "Remove " + string::highlight(this->trait->get_name()) + " Trait";
	}

private:
	trait *trait = nullptr;
};

}
