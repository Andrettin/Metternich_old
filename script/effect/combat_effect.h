#pragma once

#include "character/enemy.h"
#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "effect_list.h"
#include "engine_interface.h"
#include "random.h"
#include "script/effect/effect.h"

namespace metternich {

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

		if (key == "enemy") {
			this->enemy = enemy::get(property.get_value());
		} else if (key == "enemy_prowess") {
			this->enemy_prowess = std::stoi(property.get_value());
		} else if (key == "enemy_amount") {
			this->enemy_amount = std::stoi(property.get_value());
		} else {
			effect<T>::process_gsml_property(property);
		}
	}

	virtual void process_gsml_scope(const gsml_data &scope) override
	{
		const std::string &tag = scope.get_tag();

		if (tag == "victory") {
			this->victory_effects = std::make_unique<effect_list<T>>();
			database::process_gsml_data(this->victory_effects, scope);
		} else if (tag == "defeat") {
			this->defeat_effects = std::make_unique<effect_list<T>>();
			database::process_gsml_data(this->defeat_effects, scope);
		} else {
			effect<T>::process_gsml_scope(scope);
		}
	}

	virtual void do_assignment_effect(T *scope) const override
	{
		bool success = true;
		int enemy_amount = this->enemy_amount;
		while (enemy_amount > 0) {
			success = this->do_combat(scope);
			if (!success) {
				break;
			}
			enemy_amount--;
		}

		if (!scope->is_ai()) {
			if (success) {
				engine_interface::get()->add_notification("You won a combat.");
			} else {
				engine_interface::get()->add_notification("You lost a combat.");
			}
		}

		if (success) {
			if (this->victory_effects != nullptr) {
				this->victory_effects->do_effects(scope);
			}
		} else {
			if (this->defeat_effects != nullptr) {
				this->defeat_effects->do_effects(scope);
			}
		}
	}

	virtual std::string get_assignment_string(const T *scope, const size_t indent) const override
	{
		std::string str = "Combat against " + std::to_string(this->enemy_amount) + " ";
		if (this->enemy != nullptr) {
			str += this->enemy_amount > 1 ? this->enemy->get_name_plural() : this->enemy->get_name();
		} else {
			str += this->enemy_amount > 1 ? "enemies" : "enemy";
		}
		str += " (Prowess " + std::to_string(this->get_enemy_prowess()) + ")";
		if (this->victory_effects != nullptr) {
			const std::string effects_string = this->victory_effects->get_effects_string(scope, indent + 1);
			if (!effects_string.empty()) {
				str += "\n" + std::string(indent, '\t') + "If victorious:\n" + effects_string;
			}
		}
		if (this->defeat_effects != nullptr) {
			const std::string effects_string = this->defeat_effects->get_effects_string(scope, indent + 1);
			if (!effects_string.empty()) {
				str += "\n" + std::string(indent, '\t') + "If defeated:\n" + effects_string;
			}
		}
		return str;
	}

private:
	int get_enemy_prowess() const
	{
		if (this->enemy != nullptr) {
			return this->enemy->get_prowess();
		}

		return this->enemy_prowess;
	}

	bool do_combat(const T *scope) const
	{
		int temp_prowess = scope->get_prowess();
		int temp_enemy_prowess = this->get_enemy_prowess();

		while (temp_prowess > 0 && temp_enemy_prowess > 0) {
			const int roll_result = this->do_combat_roll(temp_prowess, temp_enemy_prowess);
			if (roll_result < 0) {
				temp_prowess += roll_result;
			} else {
				temp_enemy_prowess -= roll_result;
			}
		}

		//return true on success, and false on failure
		return temp_prowess > 0;
	}

	int do_combat_roll(const int temp_prowess, const int temp_enemy_prowess) const
	{
		return random::generate(temp_prowess) - random::generate(temp_enemy_prowess);
	}

private:
	int enemy_prowess = 0;
	int enemy_amount = 1;
	const enemy *enemy = nullptr;
	std::unique_ptr<effect_list<T>> victory_effects;
	std::unique_ptr<effect_list<T>> defeat_effects;
};

}
