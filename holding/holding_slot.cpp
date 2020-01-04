#include "holding/holding_slot.h"

#include "culture/culture.h"
#include "culture/culture_group.h"
#include "economy/commodity.h"
#include "holding/holding.h"
#include "holding/holding_slot_type.h"
#include "landed_title/landed_title.h"
#include "map/province.h"
#include "map/province_profile.h"
#include "map/region.h"
#include "random.h"
#include "religion/religion.h"
#include "religion/religion_group.h"
#include "script/chance_util.h"
#include "translator.h"
#include "util/container_util.h"

namespace metternich {

std::set<std::string> holding_slot::get_database_dependencies()
{
	return {
		//so that holding slots will be added to a province after the holding slots within the province's definition have been added to it
		province::class_identifier
	};
}

holding_slot::holding_slot(const std::string &identifier) : data_entry(identifier)
{
}

holding_slot::~holding_slot()
{
}

void holding_slot::initialize()
{
	if (this->province_profile != nullptr) {
		this->set_province(this->province_profile->get_province());
		this->province_profile = nullptr;
	}

	if (this->get_available_commodities().empty()) {
		//generate an available commodity for the holding if it has none
		this->generate_available_commodity();
	}

	data_entry_base::initialize();
}

void holding_slot::initialize_history()
{
	if (this->get_holding() != nullptr && !this->get_holding()->is_history_initialized()) {
		this->get_holding()->initialize_history();
	}

	data_entry_base::initialize_history();
}

void holding_slot::check() const
{
	if (this->get_province()->get_county() == nullptr) {
		throw std::runtime_error("The province of holding slot \"" + this->get_identifier() + "\" (\"" + this->get_province()->get_identifier() + "\") has no county.");
	}

	if (this->get_barony() != nullptr && this->get_barony()->get_de_jure_liege_title() != this->get_province()->get_county()) {
		throw std::runtime_error("The barony of holding slot \"" + this->get_identifier() + "\" is in county \"" + this->get_barony()->get_de_jure_liege_title()->get_identifier() + "\", but its province belongs to county \"" + this->get_province()->get_county()->get_identifier() + "\".");
	}
}

void holding_slot::check_history() const
{
	if (this->get_holding() != nullptr) {
		try {
			this->get_holding()->check_history();
		} catch (...) {
			std::throw_with_nested(std::runtime_error("The holding of slot \"" + this->get_identifier() + "\" is in an invalid state."));
		}
	}

	this->check();
}

std::string holding_slot::get_name() const
{
	if (this->get_barony() != nullptr) {
		return translator::get()->translate(this->get_barony()->get_identifier_with_aliases(), this->get_tag_suffix_list_with_fallbacks());
	}

	return translator::get()->translate(holding_slot_type_to_string(this->get_type()), this->get_tag_suffix_list_with_fallbacks()) + " Slot";
}

std::vector<std::vector<std::string>> holding_slot::get_tag_suffix_list_with_fallbacks() const
{
	std::vector<std::vector<std::string>> tag_list_with_fallbacks;

	if (this->get_province()->get_culture() != nullptr) {
		tag_list_with_fallbacks.push_back({this->get_province()->get_culture()->get_identifier(), this->get_province()->get_culture()->get_culture_group()->get_identifier()});
	}

	if (this->get_province()->get_religion()) {
		tag_list_with_fallbacks.push_back({this->get_province()->get_religion()->get_identifier(), this->get_province()->get_religion()->get_religion_group()->get_identifier()});
	}

	return tag_list_with_fallbacks;
}

/**
**	@brief	Get whether the holding slot is a settlement one
**
**	@return	True if the holding slot is a settlement one, or false otherwise
*/
bool holding_slot::is_settlement() const
{
	return this->get_type() == holding_slot_type::settlement;
}

/**
**	@brief	Set the holding slot's barony
**
**	@param	barony	The new barony for the holding slot
*/
void holding_slot::set_barony(landed_title *barony)
{
	if (barony == this->get_barony()) {
		return;
	}

	this->barony = barony;
	barony->set_holding_slot(this);
	emit barony_changed();
}

void holding_slot::set_province(metternich::province *province)
{
	this->province = province;
	province->add_holding_slot(this);
}

/**
**	@brief	Get the holding slot's available commodities as a QVariantList
**
**	@return	The available commodities as a QVariantList
*/
QVariantList holding_slot::get_available_commodities_qvariant_list() const
{
	return container::to_qvariant_list(this->get_available_commodities());
}

void holding_slot::set_holding(std::unique_ptr<metternich::holding> &&holding)
{
	this->holding = std::move(holding);
	emit holding_changed();
}

/**
**	@brief	Generate a commodity for the holding slot to have as available to produce
*/
void holding_slot::generate_available_commodity()
{
	std::map<metternich::commodity *, const chance_factor<holding_slot> *> commodity_chances;
	for (metternich::commodity *commodity : commodity::get_all()) {
		if (commodity->get_chance_factor() != nullptr) {
			commodity_chances[commodity] = commodity->get_chance_factor();
		}
	}

	metternich::commodity *commodity = calculate_chance_list_result(commodity_chances, this);
	this->add_available_commodity(commodity);
}

}
