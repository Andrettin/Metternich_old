#pragma once

#include "script/modifier_effect/modifier_effect.h"
#include "util/number_util.h"
#include "warfare/troop_type.h"

namespace metternich {

class troop_type;

template <typename T>
class troop_defense_modifier_effect final : public modifier_effect<T>
{
public:
	troop_defense_modifier_effect(troop_type *troop_type, const int defense)
		: troop_type(troop_type), defense(defense)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "troop_defense";
		return identifier;
	}

	virtual void apply(T *scope, const int change) const override
	{
		scope->change_troop_defense_modifier(this->troop_type, this->defense * change);
	}

	virtual std::string get_string() const override
	{
		return this->troop_type->get_name() + " Defense: " + number::to_signed_centesimal_string(this->defense);
	}

private:
	troop_type *troop_type = nullptr;
	int defense = 0;
};

}
