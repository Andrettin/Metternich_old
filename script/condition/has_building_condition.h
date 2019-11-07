#pragma once

#include "holding/building.h"
#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "script/condition/condition.h"

namespace metternich {

/**
**	@brief	A scripted has-building condition
*/
class has_building_condition : public condition
{
public:
	has_building_condition(const std::string &building_identifier)
	{
		this->building = building::get(building_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static std::string identifier = "has_building";
		return identifier;
	}

	virtual bool check(const holding *holding) const override
	{
		return holding->has_building(this->building);
	}

	virtual bool check(const holding_slot *holding_slot) const override
	{
		return holding_slot->get_holding() != nullptr && this->check(holding_slot->get_holding());
	}

private:
	building *building = nullptr;
};

}
