#pragma once

#include "script/condition/condition.h"

#include <memory>
#include <vector>

namespace metternich {

template <typename T>
class or_condition : public condition<T>
{
public:
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

	virtual bool check(const T *scope) const override
	{
		for (const std::unique_ptr<condition<T>> &condition : this->conditions) {
			if (condition->check(scope)) {
				return true;
			}
		}

		return false;
	}

	virtual void bind_condition_check(condition_check<T> &check, const T *scope) const override
	{
		for (const std::unique_ptr<condition<T>> &condition : this->conditions) {
			condition->bind_condition_check(check, scope);
		}
	}

private:
	std::vector<std::unique_ptr<condition<T>>> conditions;
};

}
