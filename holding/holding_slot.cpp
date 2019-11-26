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
#include "translator.h"
#include "util/container_util.h"

namespace metternich {

/**
**	@brief	Constructor
**
**	@param	identifier	The holding slot's identifier
*/
holding_slot::holding_slot(const std::string &identifier) : data_entry(identifier)
{
}

/**
**	@brief	Destructor
*/
holding_slot::~holding_slot()
{
}

/**
**	@brief	Initialize the holding slot
*/
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
}


/**
**	@brief	Initialize the holding slot's history
*/
void holding_slot::initialize_history()
{
	if (this->get_holding() != nullptr) {
		this->get_holding()->initialize_history();
	}
}

/**
**	@brief	Check whether the holding slot is in a valid state
*/
void holding_slot::check() const
{
	if (this->get_barony() != nullptr && this->get_barony()->get_de_jure_liege_title() != this->get_province()->get_county()) {
		throw std::runtime_error("The barony of holding slot \"" + this->get_identifier() + "\" is in county \"" + this->get_barony()->get_de_jure_liege_title()->get_identifier() + "\", but its province belongs to county \"" + this->get_province()->get_county()->get_identifier() + "\".");
	}
}

/**
**	@brief	Get the holding slot's name
**
**	@return	The holding slot's name
*/
std::string holding_slot::get_name() const
{
	if (this->get_barony() != nullptr) {
		return translator::get()->translate(this->get_barony()->get_identifier(), this->get_tag_suffix_list_with_fallbacks());
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
	return util::container_to_qvariant_list(this->get_available_commodities());
}

/**
**	@brief	Set the holding slot's holding
**
**	@param	holding	The holding
*/
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
	std::map<metternich::commodity *, std::pair<int, int>> commodity_chance_ranges;
	int total_chance_factor = 0;
	for (metternich::commodity *commodity : commodity::get_all()) {
		const int commodity_chance = commodity->calculate_chance(this);
		if (commodity_chance > 0) {
			commodity_chance_ranges[commodity] = std::pair<int, int>(total_chance_factor, total_chance_factor + commodity_chance);
			total_chance_factor += commodity_chance;
		}
	}

	if (commodity_chance_ranges.empty()) {
		return;
	}

	metternich::commodity *chosen_commodity = nullptr;

	const int random_number = random::generate(total_chance_factor);
	for (const auto &element : commodity_chance_ranges) {
		metternich::commodity *commodity = element.first;
		const std::pair<int, int> range = element.second;
		if (random_number >= range.first && random_number < range.second) {
			chosen_commodity = commodity;
		}
	}

	this->add_available_commodity(chosen_commodity);
}

}
