#pragma once

#include "script/condition/condition.h"
#include "script/condition/condition_check_base.h"
#include "script/flag/scoped_flag.h"

namespace metternich {

template <typename T>
class has_flag_condition : public condition<T>
{
public:
	has_flag_condition(const std::string &flag_identifier, const gsml_operator effect_operator)
		: condition<T>(effect_operator)
	{
		this->flag = scoped_flag<T>::get_or_add(flag_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "has_flag";
		return identifier;
	}

	virtual bool check_assignment(const T *scope) const override
	{
		return scope->has_flag(this->flag);
	}

	virtual void bind_condition_check(condition_check_base &check, const T *scope) const override
	{
		scope->connect(scope, &T::flags_changed, scope, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);
	}

private:
	const scoped_flag<T> *flag = nullptr;
};

}
