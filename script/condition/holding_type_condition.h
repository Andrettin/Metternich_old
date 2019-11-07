#pragma once

#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "holding/holding_type.h"
#include "script/condition/condition.h"

namespace metternich {

class terrain_type;

/**
**	@brief	A scripted holding type condition
*/
class holding_type_condition : public condition
{
public:
	holding_type_condition(const std::string &holding_type_identifier)
	{
		this->holding_type = holding_type::get(holding_type_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static std::string identifier = "holding_type";
		return identifier;
	}

	virtual bool check(const holding *holding) const override
	{
		return holding->get_type() == this->holding_type;
	}

	virtual bool check(const holding_slot *holding_slot) const override
	{
		return holding_slot->get_holding() != nullptr && this->check(holding_slot->get_holding());
	}

private:
	holding_type *holding_type = nullptr;
};

}
