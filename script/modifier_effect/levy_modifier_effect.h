#pragma once

#include "script/modifier_effect/modifier_effect.h"
#include "util/number_util.h"
#include "warfare/troop_type.h"

namespace metternich {

class troop_type;

template <typename T>
class levy_modifier_effect final : public modifier_effect<T>
{
public:
	levy_modifier_effect(troop_type *troop_type, const int levy)
		: troop_type(troop_type), levy(levy)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "levy";
		return identifier;
	}

	virtual void apply(T *scope, const int change) const override
	{
		scope->change_levy(this->troop_type, this->levy * change);
	}

	virtual std::string get_string() const override
	{
		return troop_type->get_name() + " Levy: " + number::to_signed_string(this->levy);
	}

private:
	troop_type *troop_type = nullptr;
	int levy = 0;
};

}
