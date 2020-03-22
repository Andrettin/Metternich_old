#include "technology/technology_slot.h"

#include "map/territory.h"
#include "script/condition/condition_check.h"
#include "script/modifier.h"
#include "technology/technology.h"
#include "util/string_util.h"

namespace metternich {

technology_slot::technology_slot(metternich::technology *technology, metternich::territory *territory)
	: technology(technology), territory(territory)
{
}

technology_slot::~technology_slot()
{
	this->precondition_check.reset();
	this->condition_check.reset();

	if (this->is_acquired()) {
		this->set_acquired(false);
	}
}

void technology_slot::initialize_history()
{
	this->create_condition_checks();
}

const std::filesystem::path &technology_slot::get_icon_path() const
{
	std::string base_tag = this->get_technology()->get_icon_tag();

	const std::filesystem::path &icon_path = database::get()->get_tagged_icon_path(base_tag, this->get_territory()->get_tag_suffix_list_with_fallbacks());
	return icon_path;
}

void technology_slot::set_available(const bool available)
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
		this->set_acquirable(false);
		this->set_acquired(false);
	}
}

void technology_slot::set_acquired(const bool acquired)
{
	if (acquired == this->is_acquired()) {
		return;
	}

	this->acquired = acquired;
	emit acquired_changed();

	if (acquired) {
		if (this->get_technology()->get_territory_modifier() != nullptr) {
			this->get_technology()->get_territory_modifier()->apply(this->get_territory());
		}

		if (this->get_technology()->get_holding_modifier() != nullptr) {
			for (holding *holding : this->get_territory()->get_holdings()) {
				this->get_technology()->get_holding_modifier()->apply(holding);
			}
		}
	} else {
		if (this->get_technology()->get_territory_modifier() != nullptr) {
			this->get_technology()->get_territory_modifier()->remove(this->get_territory());
		}

		if (this->get_technology()->get_holding_modifier() != nullptr) {
			for (holding *holding : this->get_territory()->get_holdings()) {
				this->get_technology()->get_holding_modifier()->remove(holding);
			}
		}
	}
}

void technology_slot::create_condition_checks()
{
	//create the condition checks only when initializing history, so that their result won't be calculated until history is ready
	this->precondition_check = std::make_unique<metternich::condition_check<metternich::territory>>(this->get_technology()->get_preconditions(), this->get_territory(), [this](bool result){ this->set_available(result); });
	this->condition_check = std::make_unique<metternich::condition_check<metternich::territory>>(this->get_technology()->get_conditions(), this->get_territory(), [this](bool result){ this->set_acquirable(result); });
}

QString technology_slot::get_required_technologies_string() const
{
	std::string str;

	if (!this->get_technology()->get_required_technologies().empty()) {
		str += "Required Technologies:";

		for (metternich::technology *technology : this->get_technology()->get_required_technologies()) {
			str += "\n\t(";

			if (this->get_territory()->has_technology(technology)) {
				str += string::color("*", "green");
			} else {
				str += string::color("x", "red");
			}

			str += ") " + technology->get_name();
		}
	}

	return string::to_tooltip(str);
}

QString technology_slot::get_effects_string() const
{
	std::string effects_str;

	if (this->get_technology()->get_territory_modifier() != nullptr) {
		effects_str += "Territory Modifier:\n" + this->get_technology()->get_territory_modifier()->get_string(1);
	}

	if (this->get_technology()->get_holding_modifier() != nullptr) {
		if (!effects_str.empty()) {
			effects_str += "\n";
		}

		effects_str += "Holding Modifier:\n" + this->get_technology()->get_holding_modifier()->get_string(1);
	}

	if (!effects_str.empty()) {
		return string::to_tooltip(effects_str);
	}

	return QString();
}

}
