#pragma once

#include "character/item.h"
#include "script/effect/effect.h"
#include "util/string_util.h"

namespace metternich {

class item;

template <typename T>
class items_effect : public effect<T>
{
public:
	items_effect(const std::string &item_identifier, const gsml_operator effect_operator)
		: effect<T>(effect_operator)
	{
		this->item = item::get(item_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "items";
		return identifier;
	}

	virtual void do_addition_effect(T *scope) const override
	{
		scope->add_item(this->item);
	}

	virtual void do_subtraction_effect(T *scope) const override
	{
		scope->remove_item(this->item);
	}

	virtual std::string get_addition_string() const override
	{
		return "Gain the " + string::highlight(this->item->get_name()) + " item";
	}

	virtual std::string get_subtraction_string() const override
	{
		return "Lose the " + string::highlight(this->item->get_name()) + " item";
	}

private:
	item *item = nullptr;
};

}
