#pragma once

#include "script/condition/condition.h"
#include "script/condition/condition_check.h"

namespace metternich {

template <typename T>
class alive_condition : public condition<T>
{
public:
	alive_condition(const bool alive)
	{
		this->alive = alive;
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "alive";
		return identifier;
	}

	virtual bool check(const T *scope) const override
	{
		return scope->is_alive() == this->alive;
	}

	virtual void bind_condition_check(condition_check<T> &check, const T *scope) const override
	{
		scope->connect(scope, &T::alive_changed, scope, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);
	}

private:
	bool alive = true;
};

}
