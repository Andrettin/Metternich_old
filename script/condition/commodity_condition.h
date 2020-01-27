#pragma once

#include "economy/commodity.h"
#include "script/condition/condition.h"
#include "script/condition/condition_check.h"

namespace metternich {

class commodity;

template <typename T>
class commodity_condition : public condition<T>
{
public:
	commodity_condition(const std::string &commodity_identifier)
	{
		this->commodity = commodity::get(commodity_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "commodity";
		return identifier;
	}

	virtual bool check(const T *scope) const override
	{
		//check whether the scope's commodity is the same as that for this condition
		return scope->get_commodity() == this->commodity;
	}

	virtual void bind_condition_check(condition_check<T> &check, const T *scope) const override
	{
		scope->connect(scope, &T::commodity_changed, scope, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);
	}

private:
	const metternich::commodity *commodity = nullptr;
};

}
