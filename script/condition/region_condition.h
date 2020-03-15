#pragma once

#include "map/province.h"
#include "map/region.h"
#include "script/condition/condition.h"
#include "script/scope_util.h"

namespace metternich {

class region;

template <typename T>
class region_condition final : public condition<T>
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
		const territory *territory = get_scope_territory(scope);
		return territory != nullptr && territory->is_in_region(this->region);
	}

	virtual std::string get_assignment_string() const override
	{
		return this->get_equality_string();
	}

	virtual std::string get_equality_string() const override
	{
		return "Is in the " + this->region->get_name() + " region";
	}

	virtual std::string get_inequality_string() const override
	{
		return "Is not in the " + this->region->get_name() + " region";
	}

private:
	region *region = nullptr;
};

}
