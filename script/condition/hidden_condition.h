#pragma once

#include "script/condition/and_condition.h"
#include "script/condition/condition.h"
#include "script/condition/condition_check_base.h"

namespace metternich {

template <typename T>
class hidden_condition final : public condition<T>
{
public:
	hidden_condition(const gsml_operator effect_operator = gsml_operator::assignment) : condition<T>(effect_operator)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "hidden";
		return identifier;
	}

	virtual void process_gsml_property(const gsml_property &property) override
	{
		this->conditions.process_gsml_property(property);
	}

	virtual void process_gsml_scope(const gsml_data &scope) override
	{
		this->conditions.process_gsml_scope(scope);
	}

	void add_condition(std::unique_ptr<condition<T>> &&condition)
	{
		this->conditions.add_condition(std::move(condition));
	}

	virtual bool check_assignment(const T *scope, const read_only_context &ctx) const override
	{
		return this->conditions.check(scope, ctx);
	}

	virtual void bind_condition_check(condition_check_base &check, const T *scope) const override
	{
		this->conditions.bind_condition_check(check, scope);
	}

	virtual bool is_hidden() const override
	{
		return true;
	}

private:
	and_condition<T> conditions;
};

}
