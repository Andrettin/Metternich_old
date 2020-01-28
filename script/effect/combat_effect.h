#pragma once

#include "database/gsml_property.h"
#include "engine_interface.h"
#include "random.h"
#include "script/effect/effect.h"

namespace metternich {

class gsml_data;

template <typename T>
class combat_effect : public effect<T>
{
public:
	combat_effect(const gsml_operator effect_operator) : effect<T>(effect_operator)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "combat";
		return identifier;
	}

	virtual void process_gsml_property(const gsml_property &property) override
	{
		const std::string &key = property.get_key();

		if (key == "enemy_prowess") {
			this->enemy_prowess = std::stoi(property.get_value());
		} else {
			effect<T>::process_gsml_property(property);
		}
	}

	virtual void do_assignment_effect(T *scope) const override
	{
		int temp_prowess = scope->get_prowess();
		int temp_enemy_prowess = this->enemy_prowess;

		while (temp_prowess > 0 && temp_enemy_prowess > 0) {
			const int roll_result = this->do_combat_roll(temp_prowess, temp_enemy_prowess);
			if (roll_result < 0) {
				temp_prowess += roll_result;
			} else {
				temp_enemy_prowess -= roll_result;
			}
		}

		const bool success = temp_prowess > 0;
		if (!scope->is_ai()) {
			if (success) {
				engine_interface::get()->add_notification("You won a combat.");
			} else {
				engine_interface::get()->add_notification("You lost a combat.");
			}
		}
	}

	virtual std::string get_assignment_string() const override
	{
		return "Combat against an enemy Prowess of " + std::to_string(this->enemy_prowess);
	}

	int do_combat_roll(const int temp_prowess, const int temp_enemy_prowess) const
	{
		return random::generate(temp_prowess) - random::generate(temp_enemy_prowess);
	}

private:
	int enemy_prowess = 0;
};

}
