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
void Region::InitializeHistory()
{
	this->PopulationUnits.clear();
}

QVariantList Region::GetProvincesQVariantList() const
{
	return ContainerToQVariantList(this->GetProvinces());
}

Q_INVOKABLE void Region::AddProvince(Province *province)
{
	this->Provinces.push_back(province);
	province->AddRegion(this);
}

Q_INVOKABLE void Region::RemoveProvince(Province *province)
{
	this->Provinces.erase(std::remove(this->Provinces.begin(), this->Provinces.end(), province), this->Provinces.end());
	province->RemoveRegion(this);
}

QVariantList Region::GetSubregionsQVariantList() const
{
	return ContainerToQVariantList(this->Subregions);
}

std::vector<Holding *> Region::GetHoldings() const
{
	std::vector<Holding *> holdings;

	for (const Province *province : this->GetProvinces()) {
		for (Holding *holding : province->GetHoldings()) {
			holdings.push_back(holding);
		}
	}

	return holdings;
}

/**
**	@brief	Add a population unit to the region
*/
void Region::AddPopulationUnit(std::unique_ptr<PopulationUnit> &&population_unit)
{
	this->PopulationUnits.push_back(std::move(population_unit));
}

}
