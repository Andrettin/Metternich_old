#pragma once

#include "map/province.h"
#include "map/region.h"
#include "script/condition/condition.h"
#include "script/scope_util.h"

namespace metternich {

class region;

template <typename T>
class region_condition : public condition<T>
{
public:
	region_condition(const std::string &region_identifier, const gsml_operator effect_operator)
		: condition<T>(effect_operator)
	{
		this->region = region::get(region_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "region";
		return identifier;
	}

	virtual bool check_assignment(const T *scope) const override
	{
		const province *province = get_scope_province(scope);
		return province->is_in_region(this->region);
	}

private:
	region *region = nullptr;
};

}
