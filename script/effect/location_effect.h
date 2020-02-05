#pragma once

#include "script/effect/scope_effect.h"

namespace metternich {

class province;

template <typename T>
class location_effect : public scope_effect<T, province>
{
public:
	location_effect(const gsml_operator effect_operator) : scope_effect<T, province>(effect_operator)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "location";
		return identifier;
	}

	virtual province *get_scope(const T *scope) const override
	{
		return scope->get_location();
	}
};

}
