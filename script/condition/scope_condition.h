#pragma once

#include "script/condition/and_condition.h"
#include "script/condition/condition.h"
#include "script/condition/condition_check_base.h"
#include "util/string_util.h"

namespace metternich {

//the base class for conditions that change the current scope to another one
template <typename T, typename U>
class scope_condition : public condition<T>
{
public:
	scope_condition(const gsml_operator effect_operator) : condition<T>(effect_operator)
	{
	}

	virtual void process_gsml_property(const gsml_property &property) override
	{
		this->conditions.process_gsml_property(property);
	}

	virtual void process_gsml_scope(const gsml_data &scope) override
	{
		this->conditions.process_gsml_scope(scope);
	}

	virtual U *get_scope(const T *scope) const
	{
		Q_UNUSED(scope)

		throw std::runtime_error("Scope condition type \"" + this->get_identifier() + "\" has no implementation for the function to get a scope.");
	}

	virtual U *get_scope(const T *scope, const context &ctx) const
	{
		Q_UNUSED(ctx)

		return this->get_scope(scope);
	}

	virtual bool check_assignment(const T *scope) const override
	{
		const U *new_scope = this->get_scope(scope);
		if (new_scope == nullptr) {
			return false;
		}

		return this->conditions.check(new_scope);
	}

	virtual void bind_condition_check(condition_check_base &check, const T *scope) const override
	{
		const U *new_scope = this->get_scope(scope);
		if (new_scope != nullptr) {
			this->conditions.bind_condition_check(check, new_scope);
		}
	}

	virtual std::string get_assignment_string(const T *scope, const context &ctx, const size_t indent) const override
	{
		const U *new_scope = this->get_scope(scope, ctx);

		if (new_scope == nullptr) {
			return std::string();
		}

		std::string scope_name;
		if constexpr (std::is_same_v<U, character> || std::is_same_v<U, holding>) {
			scope_name = new_scope->get_titled_name();
		} else {
			scope_name = new_scope->get_name();
		}

		std::string str = string::highlight(scope_name) + ":\n";
		str += std::string(indent, '\t') + this->conditions.get_conditions_string(new_scope, ctx, indent + 1);
		return str;
	}

private:
	and_condition<U> conditions;
};

}
