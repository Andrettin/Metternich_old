#pragma once

#include "database/database.h"
#include "database/gsml_data.h"
#include "random.h"
#include "script/effect/effect.h"
#include "script/factor_modifier.h"

#include <memory>
#include <vector>

namespace metternich {

class gsml_data;

template <typename T>
class random_list_entry
{
public:
	random_list_entry(const gsml_data &scope)
	{
		this->base_weight = std::stoi(scope.get_tag());

		for (const gsml_property &property : scope.get_properties()) {
			this->effects.push_back(effect<T>::from_gsml_property(property));
		}

		for (const gsml_data &child_scope : scope.get_children()) {
			if (child_scope.get_tag() == "modifier") {
				auto modifier = std::make_unique<factor_modifier<T>>();
				database::process_gsml_data(modifier, scope);
				this->weight_modifiers.push_back(std::move(modifier));
			} else {
				this->effects.push_back(effect<T>::from_gsml_scope(child_scope));
			}
		}
	}

	int get_weight(T *scope) const
	{
		int weight = this->base_weight;

		for (const std::unique_ptr<factor_modifier<T>> &modifier : this->weight_modifiers) {
			if (modifier->check_conditions(scope)) {
				weight *= modifier->get_factor();
				weight /= 100;
			}
		}

		return weight;
	}

	void do_effects(T *scope) const
	{
		for (const std::unique_ptr<effect<T>> &effect : this->effects) {
			effect->do_effect(scope);
		}
	}

private:
	int base_weight = 0;
	std::vector<std::unique_ptr<factor_modifier<T>>> weight_modifiers;
	std::vector<std::unique_ptr<effect<T>>> effects;
};

template <typename T>
class random_list_effect : public effect<T>
{
public:
	random_list_effect(const gsml_operator effect_operator) : effect<T>(effect_operator)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "random_list";
		return identifier;
	}

	virtual void process_gsml_scope(const gsml_data &scope) override
	{
		this->entries.emplace_back(scope);
	}

	virtual void do_assignment_effect(T *scope) const override
	{
		std::vector<const random_list_entry<T> *> weighted_entries;

		for (const random_list_entry<T> &entry : this->entries) {
			const int weight = entry.get_weight(scope);

			for (int i = 0; i < weight; ++i) {
				weighted_entries.push_back(&entry);
			}
		}

		if (!weighted_entries.empty()) {
			const random_list_entry<T> *chosen_entry = weighted_entries[random::generate(weighted_entries.size())];
			chosen_entry->do_effects(scope);
		}
	}

	virtual bool is_hidden() const override
	{
		return true;
	}

private:
	std::vector<random_list_entry<T>> entries;
};

}
