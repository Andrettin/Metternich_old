#pragma once

#include "culture/culture.h"
#include "script/condition/condition.h"
#include "script/condition/condition_check_base.h"

namespace metternich {

class culture;

template <typename T>
class culture_condition : public condition<T>
{
public:
	culture_condition(const std::string &culture_identifier)
	{
		this->culture = culture::get(culture_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "culture";
		return identifier;
	}

	virtual bool check(const T *scope) const override
	{
		return scope->get_culture() == this->culture;
	}

	virtual void bind_condition_check(condition_check_base &check, const T *scope) const override
	{
		scope->connect(scope, &T::culture_changed, scope, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);
	}

private:
	const culture *culture = nullptr;
};

}
