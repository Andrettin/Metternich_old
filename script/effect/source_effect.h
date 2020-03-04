#pragma once

#include "script/context.h"
#include "script/effect/scope_effect.h"

namespace metternich {

class character;

template <typename T>
class source_effect final : public scope_effect<T, character>
{
public:
	source_effect(const gsml_operator effect_operator) : scope_effect<T, character>(effect_operator)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "source";
		return identifier;
	}

	virtual character *get_scope(T *scope, const context &ctx) const override
	{
		Q_UNUSED(scope)

		return ctx.source_character;
	}

	virtual const character *get_scope(const T *scope, const read_only_context &ctx) const override
	{
		Q_UNUSED(scope)

		return ctx.source_character;
	}
};

}
