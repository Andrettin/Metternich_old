#pragma once

#include "database/gsml_operator.h"
#include "script/condition/condition.h"

#include <memory>
#include <vector>

namespace metternich {

template <typename T>
class and_condition final : public condition<T>
{
public:
	and_condition(const gsml_operator effect_operator = gsml_operator::assignment) : condition<T>(effect_operator)
	{
	}

	virtual void process_gsml_property(const gsml_property &property) override
	{
		std::unique_ptr<condition<T>> condition = metternich::condition<T>::from_gsml_property(property);
		this->conditions.push_back(std::move(condition));
	}

	virtual void process_gsml_scope(const gsml_data &scope) override
	{
		std::unique_ptr<condition<T>> condition = metternich::condition<T>::from_gsml_scope(scope);
		this->conditions.push_back(std::move(condition));
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "and";
		return identifier;
	}

	void add_condition(std::unique_ptr<condition<T>> &&condition)
	{
		this->conditions.push_back(std::move(condition));
	}

	virtual bool check_assignment(const T *scope, const read_only_context &ctx) const override
	{
		for (const std::unique_ptr<condition<T>> &condition : this->conditions) {
			if (!condition->check(scope, ctx)) {
				return false;
			}
		}

		return true;
	}

	virtual void bind_condition_check(condition_check_base &check, const T *scope) const override
	{
		for (const std::unique_ptr<condition<T>> &condition : this->conditions) {
			condition->bind_condition_check(check, scope);
		}
	}

	virtual std::string get_assignment_string(const T *scope, const read_only_context &ctx, const size_t indent) const
	{
		if (this->conditions.empty()) {
			return std::string();
		}

		if (this->conditions.size() == 1) {
			return this->conditions.front()->get_string(scope, ctx, indent);
		}

		std::string str = "All of these must be true:\n";
		str += this->get_conditions_string(scope, ctx, indent + 1);
		return str;
	}

	std::string get_conditions_string(const T *scope, const read_only_context &ctx, const size_t indent = 0) const
	{
		std::string conditions_string;
		bool first = true;
		for (const std::unique_ptr<condition<T>> &condition : this->conditions) {
			if (condition->is_hidden()) {
				continue;
			}

			const std::string condition_string = condition->get_string(scope, ctx, indent);
			if (condition_string.empty()) {
				continue;
			}

			if (first) {
				first = false;
			} else {
				conditions_string += "\n";
			}

			if (indent > 0) {
				conditions_string += std::string(indent, '\t');
			}

			conditions_string += condition_string;
		}
		return conditions_string;
	}

private:
	std::vector<std::unique_ptr<condition<T>>> conditions;
};

}
