#pragma once

#include "database/gsml_operator.h"
#include "script/condition/condition.h"

#include <memory>
#include <vector>

namespace metternich {

template <typename T>
class and_condition : public condition<T>
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

	virtual bool check_assignment(const T *scope) const override
	{
		for (const std::unique_ptr<condition<T>> &condition : this->conditions) {
			if (!condition->check(scope)) {
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

private:
	std::vector<std::unique_ptr<condition<T>>> conditions;
};

}
