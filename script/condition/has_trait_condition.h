#pragma once

#include "character/trait.h"
#include "script/condition/condition.h"
#include "script/condition/condition_check_base.h"

namespace metternich {

class trait;

template <typename T>
class has_trait_condition : public condition<T>
{
public:
	has_trait_condition(const std::string &trait_identifier, const gsml_operator effect_operator)
		: condition<T>(effect_operator)
	{
		this->trait = trait::get(trait_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "trait";
		return identifier;
	}

	virtual bool check_assignment(const T *scope) const override
	{
		return scope->has_trait(this->trait);
	}

	virtual void bind_condition_check(condition_check_base &check, const T *scope) const override
	{
		scope->connect(scope, &T::traits_changed, scope, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);
	}

	virtual std::string get_assignment_string() const override
	{
		return this->get_equality_string();
	}

	virtual std::string get_equality_string() const override
	{
		return "Has the " + this->trait->get_name() + " trait";
	}

	virtual std::string get_inequality_string() const override
	{
		return "Does not have the " + this->trait->get_name() + " trait";
	}

private:
	const trait *trait = nullptr;
};

}
