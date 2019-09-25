#include "map/region.h"

#include "map/province.h"
#include "population/population_unit.h"
#include "util.h"

namespace metternich {

/**
**	@brief	Constructor
*/
region::region(const std::string &identifier) : data_entry(identifier)
{
}

/**
**	@brief	Destructor
*/
region::~region()
{
}

/**
**	@brief	Initialize the region's history
*/
void region::initialize_history()
{
	this->population_units.clear();
}

QVariantList region::get_provinces_qvariant_list() const
{
	return util::container_to_qvariant_list(this->get_provinces());
}

void region::add_province(Province *province)
{
	this->provinces.push_back(province);
	province->add_region(this);
}

void region::remove_province(Province *province)
{
	this->provinces.erase(std::remove(this->provinces.begin(), this->provinces.end(), province), this->provinces.end());
	province->remove_region(this);
}

QVariantList region::get_subregions_qvariant_list() const
{
	return util::container_to_qvariant_list(this->subregions);
}

std::vector<holding *> region::get_holdings() const
{
	std::vector<holding *> holdings;

	for (const Province *province : this->get_provinces()) {
		for (holding *holding : province->get_holdings()) {
			holdings.push_back(holding);
		}
	}

	return holdings;
}

/**
**	@brief	Add a population unit to the region
*/
void region::add_population_unit(std::unique_ptr<population_unit> &&population_unit)
{
	this->population_units.push_back(std::move(population_unit));
}

}
