#pragma once

#include "script/modifier_effect/modifier_effect.h"
#include "util/number_util.h"

namespace metternich {

template <typename T>
class dueling_modifier_effect : public modifier_effect<T>
{
public:
	dueling_modifier_effect(const int dueling)
		: dueling(dueling) {}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "dueling";
		return identifier;
	}

	virtual void apply(T *scope, const int change) const override
	{
		scope->change_dueling(this->dueling * change);
	}

	virtual std::string get_string() const override
	{
		return "Dueling: " + number::to_signed_string(this->dueling);
	}

private:
	int dueling = 0;
};

}
