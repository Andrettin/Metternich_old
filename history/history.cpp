#include "history.h"

#include "character/character.h"
#include "database/database.h"
#include "engine_interface.h"
#include "holding/holding.h"
#include "landed_title/landed_title.h"
#include "map/province.h"
#include "map/region.h"
#include "population/population_unit.h"
#include "util.h"

namespace metternich {

/**
**	@brief	Load history
*/
void History::load()
{
	EngineInterface::get()->set_loading_message("Loading History...");

	region::parse_history_database();
	Province::parse_history_database();
	Character::parse_history_database();
	LandedTitle::parse_history_database();
	population_unit::parse_history_database();

	Character::process_history_database(true);

	region::process_history_database(false);
	Province::process_history_database(false);
	Character::process_history_database(false);
	LandedTitle::process_history_database(false);

	//process after the province history, so that holdings will have been created
	population_unit::process_history_database();

	History::generate_population_units();

	database::get()->initialize_history();
}

/**
**	@brief	Create population units, based on population history for regions, provinces and settlement holdings
*/
void History::generate_population_units()
{
	std::vector<population_unit *> base_population_units;

	//add population units with discount existing enabled to the vector of population units used for generation (holding-level population units with that enabled are not used for generation per se, but generated population units may still be discounted from their size), as well as any population units in provinces or regions
	for (Province *province : Province::get_all()) {
		for (holding *holding : province->get_holdings()) {
			for (const std::unique_ptr<population_unit> &population_unit : holding->get_population_units()) {
				if (population_unit->discounts_existing()) {
					base_population_units.push_back(population_unit.get());
				}
			}
		}

		for (const std::unique_ptr<population_unit> &population_unit : province->get_population_units()) {
			base_population_units.push_back(population_unit.get());
		}
	}

	for (region *region : region::get_all()) {
		for (const std::unique_ptr<population_unit> &population_unit : region->get_population_units()) {
			base_population_units.push_back(population_unit.get());
		}
	}

	//sort regions so that ones with less provinces are applied first
	std::sort(base_population_units.begin(), base_population_units.end(), [](population_unit *a, population_unit *b) {
		//give priority to population units which do not discount any type (i.e. ones that do not represent the general population, but specific population types)
		if (a->discounts_any_type() != b->discounts_any_type()) {
			return !a->discounts_any_type();
		}

		//give priority to population units which discount less types
		if (!a->discounts_any_type() && a->get_discount_types().size() != b->get_discount_types().size()) {
			return a->get_discount_types().size() < b->get_discount_types().size();
		}

		//give priority to population units located in holdings, then to provinces, then to smaller regions
		if ((a->get_holding() != nullptr) != (b->get_holding() != nullptr)) {
			return a->get_holding() != nullptr;
		} else if ((a->get_province() != nullptr) != (b->get_province() != nullptr)) {
			return a->get_province() != nullptr;
		} else if (a->get_region() != b->get_region()) {
			return a->get_region()->get_provinces().size() < b->get_region()->get_provinces().size();
		}

		return a->get_size() < b->get_size();
	});

	for (population_unit *population_unit : base_population_units) {
		//subtract the size of other population units for population units that have DiscountExisting enabled
		if (population_unit->discounts_existing()) {
			population_unit->subtract_existing_sizes();
		}

		//distribute province and region population units to the settlement holdings located in them
		if (population_unit->get_province() != nullptr) {
			population_unit->distribute_to_holdings(population_unit->get_province()->get_holdings());
		} else if (population_unit->get_region() != nullptr) {
			population_unit->distribute_to_holdings(population_unit->get_region()->get_holdings());
		}
	}

	for (population_unit *population_unit : base_population_units) {
		if (population_unit->discounts_existing()) {
			population_unit->set_discount_existing(false);
		}
	}
}

QDateTime History::StringToDate(const std::string &date_str)
{
	std::vector<std::string> date_string_list = util::split_string(date_str, '.');

	int years = 0;
	int months = 0;
	int days = 0;
	int hours = 12;

	if (date_string_list.size() >= 1) {
		years = std::stoi(date_string_list[0]);

		if (date_string_list.size() >= 2) {
			months = std::stoi(date_string_list[1]);

			if (date_string_list.size() >= 3) {
				days = std::stoi(date_string_list[2]);

				if (date_string_list.size() >= 4) {
					hours = std::stoi(date_string_list[3]);
				}
			}
		}
	}

	QDateTime date(QDate(years, months, days), QTime(hours, 0), Qt::UTC);

	if (!date.isValid()) {
		throw std::runtime_error("Date \"" + date_str + "\" is not a valid date!");
	}

	return date;
}

}
