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

namespace Metternich {

/**
**	@brief	Load history
*/
void History::Load()
{
	EngineInterface::Get()->SetLoadingMessage("Loading History...");

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

	Database::Get()->InitializeHistory();
}

/**
**	@brief	Create population units, based on population history for regions, provinces and settlement holdings
*/
void History::GeneratePopulationUnits()
{
	std::vector<PopulationUnit *> base_population_units;

	//add population units with discount existing enabled to the vector of population units used for generation (holding-level population units with that enabled are not used for generation per se, but generated population units may still be discounted from their size), as well as any population units in provinces or regions
	for (Province *province : Province::GetAll()) {
		for (Holding *holding : province->GetHoldings()) {
			for (const std::unique_ptr<PopulationUnit> &population_unit : holding->GetPopulationUnits()) {
				if (population_unit->DiscountsExisting()) {
					base_population_units.push_back(population_unit.get());
				}
			}
		}

		for (const std::unique_ptr<PopulationUnit> &population_unit : province->GetPopulationUnits()) {
			base_population_units.push_back(population_unit.get());
		}
	}

	for (Region *region : Region::GetAll()) {
		for (const std::unique_ptr<PopulationUnit> &population_unit : region->GetPopulationUnits()) {
			base_population_units.push_back(population_unit.get());
		}
	}

	//sort regions so that ones with less provinces are applied first
	std::sort(base_population_units.begin(), base_population_units.end(), [](PopulationUnit *a, PopulationUnit *b) {
		//give priority to population units which do not discount any type (i.e. ones that do not represent the general population, but specific population types)
		if (a->DiscountsAnyType() != b->DiscountsAnyType()) {
			return !a->DiscountsAnyType();
		}

		//give priority to population units which discount less types
		if (!a->DiscountsAnyType() && a->GetDiscountTypes().size() != b->GetDiscountTypes().size()) {
			return a->GetDiscountTypes().size() < b->GetDiscountTypes().size();
		}

		//give priority to population units located in holdings, then to provinces, then to smaller regions
		if ((a->GetHolding() != nullptr) != (b->GetHolding() != nullptr)) {
			return a->GetHolding() != nullptr;
		} else if ((a->GetProvince() != nullptr) != (b->GetProvince() != nullptr)) {
			return a->GetProvince() != nullptr;
		} else if (a->GetRegion() != b->GetRegion()) {
			return a->GetRegion()->GetProvinces().size() < b->GetRegion()->GetProvinces().size();
		}

		return a->GetSize() < b->GetSize();
	});

	for (PopulationUnit *population_unit : base_population_units) {
		//subtract the size of other population units for population units that have DiscountExisting enabled
		if (population_unit->DiscountsExisting()) {
			population_unit->SubtractExistingSizes();
		}

		//distribute province and region population units to the settlement holdings located in them
		if (population_unit->GetProvince() != nullptr) {
			population_unit->DistributeToHoldings(population_unit->GetProvince()->GetHoldings());
		} else if (population_unit->GetRegion() != nullptr) {
			population_unit->DistributeToHoldings(population_unit->GetRegion()->GetHoldings());
		}
	}

	for (PopulationUnit *population_unit : base_population_units) {
		if (population_unit->DiscountsExisting()) {
			population_unit->SetDiscountExisting(false);
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
