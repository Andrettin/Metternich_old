#include "holding/holding_slot.h"

#include "economy/commodity.h"
#include "landed_title/landed_title.h"
#include "random.h"

namespace metternich {

/**
**	@brief	Initialize the holding slot
*/
void holding_slot::initialize()
{
	if (this->get_commodity() == nullptr) {
		//generate a commodity for the holding if it has none
		this->generate_commodity();
	}
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

/**
**	@brief	Generate a commodity for the holding slot to have as available to produce
*/
void holding_slot::generate_commodity()
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

	const int random_number = Random::generate(total_chance_factor);
	for (const auto &element : commodity_chance_ranges) {
		metternich::commodity *commodity = element.first;
		const std::pair<int, int> range = element.second;
		if (random_number >= range.first && random_number < range.second) {
			chosen_commodity = commodity;
		}
	}

	this->set_commodity(chosen_commodity);
}

}
