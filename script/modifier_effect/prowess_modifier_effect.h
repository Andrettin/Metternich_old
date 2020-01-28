#pragma once

#include "script/modifier_effect/modifier_effect.h"
#include "util/number_util.h"

namespace metternich {

template <typename T>
class prowess_modifier_effect : public modifier_effect<T>
{
public:
	prowess_modifier_effect(const int prowess)
		: prowess(prowess) {}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "prowess";
		return identifier;
	}

	virtual void apply(T *scope, const int change) const override
	{
		scope->change_prowess(this->prowess * change);
	}

	virtual std::string get_string() const override
	{
		return "Prowess: " + number::to_signed_string(this->prowess);
	}

private:
	int prowess = 0;
};

}
