#pragma once

#include "script/condition/condition.h"

#include <memory>
#include <vector>

namespace metternich {

template <typename T>
class not_condition : public condition<T>
{
public:
	not_condition(const gsml_operator effect_operator) : condition<T>(effect_operator)
	{
	}

	not_condition(std::unique_ptr<condition<T>> &&enclosed_condition, const gsml_operator effect_operator)
		: condition<T>(effect_operator)
	{
		this->conditions.push_back(std::move(enclosed_condition));
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
		static const std::string identifier = "not";
		return identifier;
	}

	virtual bool check_assignment(const T *scope) const override
	{
		for (const std::unique_ptr<condition<T>> &condition : this->conditions) {
			if (condition->check(scope)) {
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

	virtual std::string get_assignment_string(const T *scope, const context &ctx, const size_t indent) const
	{
		if (this->conditions.empty()) {
			return std::string();
		}

		std::string str = "None of these must be true:\n";
		const size_t subcondition_indent = indent + 1;
		bool first = true;
		for (const std::unique_ptr<condition<T>> &condition : this->conditions) {
			if (condition->is_hidden()) {
				continue;
			}

			const std::string condition_string = condition->get_string(scope, ctx, subcondition_indent);
			if (condition_string.empty()) {
				continue;
			}

			if (first) {
				first = false;
			} else {
				str += "\n";
			}

			if (subcondition_indent > 0) {
				str += std::string(subcondition_indent, '\t');
			}

			str += condition_string;
		}
		return str;
	}

private:
	std::vector<std::unique_ptr<condition<T>>> conditions;
};

}
