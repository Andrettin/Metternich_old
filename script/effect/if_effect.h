#pragma once

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "script/condition/and_condition.h"
#include "script/effect/effect.h"
#include "script/effect/effect_list.h"

namespace metternich {

template <typename T>
class if_effect : public effect<T>
{
public:
	if_effect(const gsml_operator effect_operator) : effect<T>(effect_operator)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "if";
		return identifier;
	}

	virtual void process_gsml_property(const gsml_property &property) override
	{
		this->effects.process_gsml_property(property);
	}

	virtual void process_gsml_scope(const gsml_data &scope) override
	{
		if (scope.get_tag() == "conditions") {
			database::process_gsml_data(this->conditions, scope);
		} else {
			this->effects.process_gsml_scope(scope);
		}
	}

	virtual void do_assignment_effect(T *scope, const context &ctx) const override
	{
		if (!this->conditions.check(scope, ctx)) {
			return;
		}

		this->effects.do_effects(scope, ctx);
	}

	virtual std::string get_assignment_string(const T *scope, const read_only_context &ctx, const size_t indent) const override
	{
		if (!this->conditions.check(scope, ctx)) {
			return std::string();
		}

		return this->effects.get_effects_string(scope, ctx, indent);
	}

private:
	and_condition<T> conditions;
	effect_list<T> effects;
};

}
