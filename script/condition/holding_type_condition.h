#pragma once

#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "holding/holding_type.h"
#include "script/condition/condition.h"
#include "script/condition/condition_check.h"

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

	virtual void bind_condition_check(condition_check<holding> &check, const holding *holding) const override
	{
		holding->connect(holding, &holding::type_changed, holding, [&check](){ check.set_result_recalculation_needed(); }, Qt::ConnectionType::DirectConnection);
	}

private:
	holding_type *holding_type = nullptr;
};

}
