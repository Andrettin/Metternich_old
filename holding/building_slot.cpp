#include "holding/building_slot.h"

#include "holding/building.h"
#include "holding/holding.h"
#include "script/condition/condition_check.h"

namespace metternich {

building_slot::building_slot(metternich::building *building, metternich::holding *holding)
	: building(building), holding(holding)
{
	this->precondition_check = std::make_unique<metternich::condition_check<metternich::holding>>(building->get_preconditions(), holding, [this](bool result){ this->set_available(result); });
	this->condition_check = std::make_unique<metternich::condition_check<metternich::holding>>(building->get_conditions(), holding, [this](bool result){ this->set_buildable(result); });
}

building_slot::~building_slot()
{
	this->precondition_check.reset();
	this->condition_check.reset();

	if (this->is_built()) {
		this->set_built(false);
	}
}

const std::filesystem::path &building_slot::get_icon_path() const
{
	std::string base_tag = this->get_building()->get_icon_tag();

	const std::filesystem::path &icon_path = database::get()->get_tagged_icon_path(base_tag, this->holding->get_tag_suffix_list_with_fallbacks());
	return icon_path;
}

void building_slot::set_available(const bool available)
{
	if (available == this->is_available()) {
		return;
	}

	this->available = available;
	emit available_changed();

	if (available) {
		//the building slot has become available, check whether the building is actually buildable
		if (this->condition_check != nullptr) {
			this->condition_check->calculate_result();
		}
	} else {
		this->set_buildable(false);
		this->set_built(false);
	}
}

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
