#pragma once

#include "holding/holding.h"
#include "holding/holding_type.h"
#include "script/condition/condition.h"
#include "script/condition/condition_check.h"

namespace metternich {

/**
**	@brief	A scripted holding type condition
*/
template <typename T>
class holding_type_condition : public condition<T>
{
public:
	holding_type_condition(const std::string &holding_type_identifier)
	{
		this->holding_type = holding_type::get(holding_type_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "holding_type";
		return identifier;
	}

	virtual bool check(const T *scope) const override
	{
		return scope->get_type() == this->holding_type;
	}

	virtual void bind_condition_check(condition_check<T> &check, const T *scope) const override
	{
		scope->connect(scope, &T::type_changed, scope, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);
	}

private:
	const holding_type *holding_type = nullptr;
};

}
