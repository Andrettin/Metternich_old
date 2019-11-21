#pragma once

#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "map/province.h"
#include "map/region.h"
#include "script/condition/condition.h"

namespace metternich {

class region;

/**
**	@brief	A scripted region condition
*/
class region_condition : public condition
{
public:
	region_condition(const std::string &region_identifier)
	{
		this->region = region::get(region_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static std::string identifier = "region";
		return identifier;
	}

	virtual bool check(const province *province) const override
	{
		return province->is_in_region(this->region);
	}

	virtual bool check(const holding *holding) const override
	{
		return this->check(holding->get_province());
	}

	virtual bool check(const holding_slot *holding_slot) const override
	{
		return this->check(holding_slot->get_province());
	}

private:
	region *region = nullptr;
};

}
