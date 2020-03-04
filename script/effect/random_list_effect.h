#pragma once

#include "database/database.h"
#include "database/gsml_data.h"
#include "random.h"
#include "script/effect/effect.h"
#include "script/effect/effect_list.h"
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
		this->effects = std::make_unique<effect_list<T>>();

		scope.for_each_element([&](const gsml_property &property) {
			this->effects->process_gsml_property(property);
		}, [&](const gsml_data &child_scope) {
			if (child_scope.get_tag() == "modifier") {
				auto modifier = std::make_unique<factor_modifier<T>>();
				database::process_gsml_data(modifier, child_scope);
				this->weight_modifiers.push_back(std::move(modifier));
			} else {
				this->effects->process_gsml_scope(child_scope);
			}
		});
	}

	int get_weight(const T *scope, const read_only_context &ctx) const
	{
		int weight = this->base_weight;

		for (const std::unique_ptr<factor_modifier<T>> &modifier : this->weight_modifiers) {
			if (modifier->check_conditions(scope, ctx)) {
				weight *= modifier->get_factor();
				weight /= 100;
			}
		}

		return weight;
	}

	void do_effects(T *scope, const context &ctx) const
	{
		this->effects->do_effects(scope, ctx);
	}

	std::string get_effects_string(const T *scope, const read_only_context &ctx, const size_t indent) const
	{
		std::string effects_string = this->effects->get_effects_string(scope, ctx, indent);

		if (!effects_string.empty()) {
			return effects_string;
		}

		return std::string(indent, '\t') + no_effect_string;
	}

private:
	int base_weight = 0;
	std::vector<std::unique_ptr<factor_modifier<T>>> weight_modifiers;
	std::unique_ptr<effect_list<T>> effects;
};

template <typename T>
class random_list_effect final : public effect<T>
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

	virtual void do_assignment_effect(T *scope, const context &ctx) const override
	{
		const std::vector<const random_list_entry<T> *> weighted_entries = this->get_weighted_entries(scope, ctx);

		if (!weighted_entries.empty()) {
			const random_list_entry<T> *chosen_entry = weighted_entries[random::generate(weighted_entries.size())];
			chosen_entry->do_effects(scope, ctx);
		}
	}

	virtual std::string get_assignment_string(const T *scope, const read_only_context &ctx, const size_t indent) const override
	{
		int total_weight = 0;
		std::vector<std::pair<const random_list_entry<T> *, int>> entry_weights;
		for (const random_list_entry<T> &entry : this->entries) {
			const int weight = entry.get_weight(scope, ctx);
			if (weight > 0) {
				total_weight += weight;
				entry_weights.emplace_back(&entry, weight);
			}
		}

		if (total_weight == 0) {
			return std::string();
		} else if (entry_weights.size() == 1) {
			return (*entry_weights.begin()).first->get_effects_string(scope, ctx, indent);
		}

		std::string str = "One of these will occur:\n";

		bool first = true;
		for (const auto &entry_weight_pair : entry_weights) {
			const random_list_entry<T> *entry = entry_weight_pair.first;
			const int weight = entry_weight_pair.second;

			if (first) {
				first = false;
			} else {
				str += "\n";
			}

			str += std::string(indent + 1, '\t');

			const int chance = weight * 100 / total_weight;
			const std::string effects_string = entry->get_effects_string(scope, ctx, indent + 2);
			str += std::to_string(chance) + "% chance of:\n" + effects_string;
		}

		return str;
	}

private:
	std::vector<const random_list_entry<T> *> get_weighted_entries(T *scope, const context &ctx) const
	{
		std::vector<const random_list_entry<T> *> weighted_entries;

		for (const random_list_entry<T> &entry : this->entries) {
			const int weight = entry.get_weight(scope, ctx);

			for (int i = 0; i < weight; ++i) {
				weighted_entries.push_back(&entry);
			}
		}

		return weighted_entries;
	}

private:
	std::vector<random_list_entry<T>> entries;
};

}
