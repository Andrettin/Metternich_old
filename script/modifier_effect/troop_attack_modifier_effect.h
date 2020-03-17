#pragma once

#include "script/modifier_effect/modifier_effect.h"
#include "util/number_util.h"
#include "warfare/troop_type.h"

namespace metternich {

class troop_type;

template <typename T>
class troop_attack_modifier_effect final : public modifier_effect<T>
{
public:
	troop_attack_modifier_effect(troop_type *troop_type, const int attack)
		: troop_type(troop_type), attack(attack)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "troop_attack";
		return identifier;
	}

	virtual void apply(T *scope, const int change) const override
	{
		scope->change_troop_attack_modifier(this->troop_type, this->attack * change);
	}

	virtual std::string get_string() const override
	{
		return this->troop_type->get_name() + " Attack: " + number::to_signed_string(this->attack);
	}

private:
	troop_type *troop_type = nullptr;
	int attack = 0;
};

}
