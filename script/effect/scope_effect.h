#pragma once

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "script/effect/effect.h"
#include "script/effect/effect_list.h"
#include "util/string_util.h"

namespace metternich {

//the base class for effects that change the current scope to another one
template <typename T, typename U>
class scope_effect : public effect<T>
{
public:
	scope_effect(const gsml_operator effect_operator) : effect<T>(effect_operator)
	{
	}

	virtual void process_gsml_property(const gsml_property &property) override
	{
		this->effects.process_gsml_property(property);
	}

	virtual void process_gsml_scope(const gsml_data &scope) override
	{
		this->effects.process_gsml_scope(scope);
	}

	virtual U *get_scope(const T *scope) const
	{
		Q_UNUSED(scope)

		throw std::runtime_error("Scope effect type \"" + this->get_identifier() + "\" has no implementation for the function to get a scope.");
	}

	virtual U *get_scope(const T *scope, const context &ctx) const
	{
		Q_UNUSED(ctx)

		return this->get_scope(scope);
	}

	virtual void do_assignment_effect(T *scope, const context &ctx) const override
	{
		U *new_scope = this->get_scope(scope, ctx);

		if (new_scope == nullptr) {
			throw std::runtime_error("Scope effect \"" + this->get_identifier() + "\" has a null scope.");
		}

		this->effects.do_effects(new_scope, ctx);
	}

	virtual std::string get_assignment_string(const T *scope, const context &ctx, const size_t indent) const override
	{
		const U *new_scope = this->get_scope(scope, ctx);

		if (new_scope == nullptr) {
			throw std::runtime_error("Scope effect \"" + this->get_identifier() + "\" has a null scope.");
		}

		std::string scope_name;
		if constexpr (std::is_same_v<U, character> || std::is_same_v<U, holding>) {
			scope_name = new_scope->get_titled_name();
		} else {
			scope_name = new_scope->get_name();
		}

		std::string str = string::highlight(scope_name) + ":\n";
		str += std::string(indent, '\t') + this->effects.get_effects_string(new_scope, ctx, indent + 1);
		return str;
	}

private:
	effect_list<U> effects;
};

}
