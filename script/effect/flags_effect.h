#pragma once

#include "script/effect/effect.h"
#include "script/flag/scoped_flag.h"

namespace metternich {

class trait;

template <typename T>
class flags_effect final : public effect<T>
{
public:
	flags_effect(const std::string &flag_identifier, const gsml_operator effect_operator)
		: effect<T>(effect_operator)
	{
		this->flag = scoped_flag<T>::get_or_add(flag_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "flags";
		return identifier;
	}

	virtual void do_addition_effect(T *scope) const override
	{
		scope->add_flag(this->flag);
	}

	virtual void do_subtraction_effect(T *scope) const override
	{
		scope->remove_flag(this->flag);
	}

	virtual bool is_hidden() const override
	{
		return true;
	}

private:
	const scoped_flag<T> *flag = nullptr;
};

}
