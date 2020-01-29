#pragma once

#include "politics/law.h"
#include "script/condition/condition.h"
#include "script/condition/condition_check_base.h"

namespace metternich {

template <typename T>
class has_law_condition : public condition<T>
{
public:
	has_law_condition(const std::string &law_identifier)
	{
		this->law = law::get(law_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "has_law";
		return identifier;
	}

	virtual bool check(const T *scope) const override
	{
		return scope->has_law(this->law);
	}

	virtual void bind_condition_check(condition_check_base &check, const T *scope) const override
	{
		scope->connect(scope, &T::laws_changed, scope, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);
	}

private:
	law *law = nullptr;
};

}
