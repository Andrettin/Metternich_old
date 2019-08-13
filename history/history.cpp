#include "history.h"

#include "character/character.h"
#include "holding/holding.h"
#include "landed_title/landed_title.h"
#include "map/province.h"
#include "map/region.h"
#include "population/population_unit.h"
#include "util.h"

namespace Metternich {

/**
**	@brief	Load history
*/
void History::Load()
{
	Region::ParseHistoryDatabase();
	Province::ParseHistoryDatabase();
	Character::ParseHistoryDatabase();
	LandedTitle::ParseHistoryDatabase();
	PopulationUnit::ParseHistoryDatabase();

	Character::ProcessHistoryDatabase(true);

	Region::ProcessHistoryDatabase(false);
	Province::ProcessHistoryDatabase(false);
	Character::ProcessHistoryDatabase(false);
	LandedTitle::ProcessHistoryDatabase(false);

	//process after the province history, so that holdings will have been created
	PopulationUnit::ProcessHistoryDatabase();

	History::GeneratePopulationUnits();

	Province::InitializeAll();
	Region::InitializeAll();
	Character::InitializeAll();
	LandedTitle::InitializeAll();

	Character::CheckAll();
	LandedTitle::CheckAll();
	Province::CheckAll();
	Region::CheckAll();
}

/**
**	@brief	Create population units, based on population history for regions, provinces and settlement holdings
*/
void History::GeneratePopulationUnits()
{
	for (Province *province : Province::GetAll()) {
		//subtract the size of other population units for population units that have DiscountExisting enabled
		for (Holding *holding : province->GetHoldings()) {
			for (const std::unique_ptr<PopulationUnit> &population_unit : holding->GetPopulationUnits()) {
				if (population_unit->DiscountsExisting()) {
					population_unit->SubtractExistingSizes();
					population_unit->SetDiscountExisting(false);
				}
			}
		}

		for (const std::unique_ptr<PopulationUnit> &population_unit : province->GetPopulationUnits()) {
			if (population_unit->DiscountsExisting()) {
				population_unit->SubtractExistingSizes();
			}
		}

		//distribute province population units to the settlement holdings in that province
		for (const std::unique_ptr<PopulationUnit> &population_unit : province->GetPopulationUnits()) {
			population_unit->DistributeToHoldings(province->GetHoldings());
		}
	}

	std::vector<Region *> regions = Region::GetAll();

	//sort regions so that ones with less provinces are applied first
	std::sort(regions.begin(), regions.end(), [](Region *a, Region *b) {
		return a->GetProvinces().size() < b->GetProvinces().size();
	});

	//do regions after provinces, so that they have lower priority
	for (Region *region : regions) {
		for (const std::unique_ptr<PopulationUnit> &population_unit : region->GetPopulationUnits()) {
			if (population_unit->DiscountsExisting()) {
				population_unit->SubtractExistingSizes();
			}
		}

		//distribute province population units to the settlement holdings in that province
		for (const std::unique_ptr<PopulationUnit> &population_unit : region->GetPopulationUnits()) {
			population_unit->DistributeToHoldings(region->GetHoldings());
		}
	}
}

QDateTime History::StringToDate(const std::string &date_str)
{
	std::vector<std::string> date_string_list = SplitString(date_str, '.');

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
