#pragma once

#include "script/condition/condition.h"
#include "script/condition/condition_check_base.h"

namespace metternich {

template <typename T>
class ai_condition : public condition<T>
{
public:
	ai_condition(const bool ai, const gsml_operator effect_operator) : condition<T>(effect_operator), ai(ai)
	{
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "ai";
		return identifier;
	}

	virtual bool check_assignment(const T *scope) const override
	{
		return this->check_equality(scope);
	}

	virtual bool check_equality(const T *scope) const override
	{
		return scope->is_ai() == this->ai;
	}

	virtual void bind_condition_check(condition_check_base &check, const T *scope) const override
	{
		scope->connect(scope, &T::ai_changed, scope, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);
	}

	virtual std::string get_assignment_string() const override
	{
		return this->get_equality_string();
	}

	virtual std::string get_equality_string() const override
	{
		return "Is " + std::string(this->ai ? "" : "not") + " AI-controlled";
	}

	virtual std::string get_inequality_string() const override
	{
		return "Is " + std::string(!this->ai ? "" : "not") + " AI-controlled";
	}

	virtual bool is_hidden() const override
	{
		return true;
	}

private:
	bool ai = true;
};

}
