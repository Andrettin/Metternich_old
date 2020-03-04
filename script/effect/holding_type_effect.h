#pragma once

#include "holding/holding_type.h"
#include "script/effect/effect.h"
#include "util/string_util.h"

namespace metternich {

template <typename T>
class holding_type_effect final : public effect<T>
{
public:
	holding_type_effect(const std::string &value, const gsml_operator effect_operator)
		: effect<T>(effect_operator)
	{
		this->holding_type = holding_type::get(value);
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "holding_type";
		return identifier;
	}

	virtual void do_assignment_effect(T *scope) const override
	{
		scope->set_type(this->holding_type);
	}

	virtual std::string get_assignment_string() const override
	{
		return "Set holding type to " + string::highlight(this->holding_type->get_name());
	}

private:
	holding_type *holding_type = nullptr;
};

}
