#pragma once

#include "character/item.h"
#include "script/condition/condition.h"
#include "script/condition/condition_check_base.h"

namespace metternich {

class item;

template <typename T>
class has_item_condition : public condition<T>
{
public:
	has_item_condition(const std::string &item_identifier, const gsml_operator effect_operator)
		: condition<T>(effect_operator)
	{
		this->item = item::get(item_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "has_item";
		return identifier;
	}

	virtual bool check_assignment(const T *scope) const override
	{
		return scope->has_item(this->item);
	}

	virtual void bind_condition_check(condition_check_base &check, const T *scope) const override
	{
		scope->connect(scope, &T::items_changed, scope, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);
	}

	virtual std::string get_assignment_string() const override
	{
		return this->get_equality_string();
	}

	virtual std::string get_equality_string() const override
	{
		return "Has the " + this->item->get_name() + " item";
	}

	virtual std::string get_inequality_string() const override
	{
		return "Does not have the " + this->item->get_name() + " item";
	}

private:
	const item *item = nullptr;
};

}
