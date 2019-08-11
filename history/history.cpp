#include "history.h"

#include "character/character.h"
#include "holding/holding.h"
#include "landed_title/landed_title.h"
#include "map/province.h"
#include "map/region.h"
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

	Character::ProcessHistoryDatabase(true);

	Region::ProcessHistoryDatabase(false);
	Province::ProcessHistoryDatabase(false);
	Character::ProcessHistoryDatabase(false);
	LandedTitle::ProcessHistoryDatabase(false);

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
	std::vector<Region *> regions = Region::GetAll();

	//sort regions so that ones with less provinces are applied first
	std::sort(regions.begin(), regions.end(), [](Region *a, Region *b) {
		return a->GetProvinces().size() < b->GetProvinces().size();
	});

	for (Province *province : Province::GetAll()) {
		//use province population counts to set the population count for their settlement holdings
		if (province->GetPopulation() != 0) {
			History::SetPopulationForHoldings(province->GetPopulation(), province->GetHoldings());
			province->SetPopulation(0);
		}
	}

	//do regions after provinces, so that they have lower priority
	for (Region *region : regions) {
		if (region->GetPopulation() != 0) {
			History::SetPopulationForHoldings(region->GetPopulation(), region->GetHoldings());
			region->SetPopulation(0);
		}
	}

	for (Province *province : Province::GetAll()) {
		//generate population units for holdings
		for (Holding *holding : province->GetHoldings()) {
			holding->GeneratePopulationUnits();
		}
	}
}

/**
**	@brief	Set a population number for a group of settlement holdings
**
**	@param	population	The population size
**	@param	holdings	The group of settlement holdings
*/
void History::SetPopulationForHoldings(int population, const std::vector<Holding *> &holdings)
{
	//set population for settlement holdings without population data
	int holding_count = 0; //count of settlement holdings for which the population will be applied

	//first, remove the population count for settlement holdings which do have it set from the population count
	for (const Holding *holding : holdings) {
		if (holding->GetPopulation() == 0) {
			holding_count++;
			continue;
		}

		population -= holding->GetPopulation();
	}

	if (population <= 0 || holding_count == 0) {
		return;
	}

	//now, apply the remaining population to all settlement holdings without population set for them, in equal proportions
	population /= holding_count;
	for (Holding *holding : holdings) {
		if (holding->GetPopulation() == 0) {
			holding->SetPopulation(population);
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
