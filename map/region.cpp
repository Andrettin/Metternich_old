#include "map/region.h"

#include "map/province.h"
#include "population/population_unit.h"
#include "util.h"

namespace metternich {

/**
**	@brief	Constructor
*/
Region::Region(const std::string &identifier) : DataEntry(identifier)
{
}

/**
**	@brief	Destructor
*/
Region::~Region()
{
}

/**
**	@brief	Initialize the region's history
*/
void Region::initialize_history()
{
	this->population_units.clear();
}

QVariantList Region::GetProvincesQVariantList() const
{
	return util::container_to_qvariant_list(this->GetProvinces());
}

void Region::add_province(Province *province)
{
	this->Provinces.push_back(province);
	province->AddRegion(this);
}

void Region::remove_province(Province *province)
{
	this->Provinces.erase(std::remove(this->Provinces.begin(), this->Provinces.end(), province), this->Provinces.end());
	province->RemoveRegion(this);
}

QVariantList Region::GetSubregionsQVariantList() const
{
	return util::container_to_qvariant_list(this->Subregions);
}

std::vector<holding *> Region::get_holdings() const
{
	std::vector<holding *> holdings;

	for (const Province *province : this->GetProvinces()) {
		for (holding *holding : province->get_holdings()) {
			holdings.push_back(holding);
		}
	}

	return holdings;
}

/**
**	@brief	Add a population unit to the region
*/
void Region::add_population_unit(std::unique_ptr<population_unit> &&population_unit)
{
	this->population_units.push_back(std::move(population_unit));
}

}
