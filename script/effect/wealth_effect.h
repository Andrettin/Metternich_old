#pragma once

#include "script/effect/effect.h"
#include "util/number_util.h"
#include "util/parse_util.h"

namespace metternich {

template <typename T>
class wealth_effect final : public effect<T>
{
public:
	wealth_effect(const std::string &value, const gsml_operator effect_operator)
		: effect<T>(effect_operator)
	{
		this->wealth = parse::centesimal_number_string_to_int(value);
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "wealth";
		return identifier;
	}

	virtual void do_assignment_effect(T *scope) const override
	{
		scope->set_wealth(this->wealth);
	}

	virtual void do_addition_effect(T *scope) const override
	{
		scope->change_wealth(this->wealth);
	}

	virtual void do_subtraction_effect(T *scope) const override
	{
		scope->change_wealth(-this->wealth);
	}

	virtual std::string get_assignment_string() const override
	{
		return "Set Wealth to " + number::to_centesimal_string(this->wealth);
	}

	virtual std::string get_addition_string() const override
	{
		return number::to_signed_centesimal_string(this->wealth) + " Wealth";
	}

	virtual std::string get_subtraction_string() const override
	{
		return number::to_signed_centesimal_string(-this->wealth) + " Wealth";
	}

private:
	int wealth = 0;
};

}
