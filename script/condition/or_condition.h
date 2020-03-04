#pragma once

#include "script/condition/condition.h"

#include <memory>
#include <vector>

namespace metternich {

template <typename T>
class or_condition final : public condition<T>
{
public:
	or_condition(const gsml_operator effect_operator) : condition<T>(effect_operator)
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
		static const std::string identifier = "or";
		return identifier;
	}

	virtual bool check_assignment(const T *scope, const read_only_context &ctx) const override
	{
		for (const std::unique_ptr<condition<T>> &condition : this->conditions) {
			if (condition->check(scope, ctx)) {
				return true;
			}
		}

		return false;
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

		std::string str = "One of these must be true:\n";
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
