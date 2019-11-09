#include "holding/building_slot.h"

#include "holding/building.h"
#include "holding/holding.h"
#include "script/condition/condition_check.h"

namespace metternich {

/**
**	@brief	Constructor
*/
building_slot::building_slot(metternich::building *building, metternich::holding *holding)
	: building(building), holding(holding)
{
	this->precondition_check = std::make_unique<metternich::condition_check<metternich::holding>>(building->get_preconditions(), holding, [this](bool result){ this->set_available(result); });
	this->condition_check = std::make_unique<metternich::condition_check<metternich::holding>>(building->get_conditions(), holding, [this](bool result){ this->set_buildable(result); });
}

/**
**	@brief	Destructor
*/
building_slot::~building_slot()
{
	if (this->is_built()) {
		this->set_built(false);
	}
}

/**
**	@brief	Set whether the building is constructed
*/
void building_slot::set_built(const bool built)
{
	if (built == this->is_built()) {
		return;
	}

	this->built = built;

	this->holding->apply_building_effects(this->get_building(), built ? 1 : -1);
	emit built_changed();
}

}
