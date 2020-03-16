#include "holding/building_slot.h"

#include "economy/employment.h"
#include "economy/employment_type.h"
#include "holding/building.h"
#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "script/condition/condition_check.h"
#include "util/number_util.h"
#include "util/string_util.h"

namespace metternich {

building_slot::building_slot(metternich::building *building, metternich::holding *holding)
	: building(building), holding(holding)
{
}

building_slot::~building_slot()
{
	this->precondition_check.reset();
	this->condition_check.reset();

	if (this->is_built()) {
		this->set_built(false);
	}
}

void building_slot::initialize_history()
{
	this->create_condition_checks();
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
	emit built_changed();

	if (this->get_building()->get_employment_type() != nullptr) {
		if (built) {
			this->employment = std::make_unique<metternich::employment>(this->get_building()->get_employment_type(), this);

			long long int workforce_capacity = this->get_building()->get_workforce();
			workforce_capacity *= this->holding->get_holding_size();
			workforce_capacity /= holding_slot::default_holding_size;
			this->employment->set_workforce_capacity(static_cast<int>(workforce_capacity));
			this->holding->add_employment(this->employment.get());
		} else {
			this->holding->remove_employment(this->employment.get());
			this->employment.reset();
		}
	}
}

int building_slot::get_workforce() const
{
	if (this->employment != nullptr) {
		return this->employment->get_workforce();
	}

	return 0;
}

int building_slot::get_workforce_capacity() const
{
	if (this->employment != nullptr) {
		return this->employment->get_workforce_capacity();
	}

	return 0;
}

void building_slot::create_condition_checks()
{
	//create the condition checks only when initializing history, so that their result won't be calculated until history is ready
	this->precondition_check = std::make_unique<metternich::condition_check<metternich::holding>>(this->get_building()->get_preconditions(), this->holding, [this](bool result){ this->set_available(result); });
	this->condition_check = std::make_unique<metternich::condition_check<metternich::holding>>(this->get_building()->get_conditions(), this->holding, [this](bool result){ this->set_buildable(result); });
}

QString building_slot::get_effects_string() const
{
	std::string effects_str;

	if (this->get_building()->get_employment_type() != nullptr) {
		long long int workforce = this->get_building()->get_workforce();
		workforce *= this->holding->get_holding_size();
		workforce /= holding_slot::default_holding_size;
		effects_str += this->get_building()->get_employment_type()->get_name() + " Employment Capacity: " + number::to_signed_string(static_cast<int>(workforce));
	}

	if (!effects_str.empty()) {
		return string::to_tooltip(effects_str);
	}

	return QString();
}

}
