#include "holding/building_slot.h"

#include "holding/holding.h"

namespace metternich {

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
