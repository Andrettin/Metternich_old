#include "map/region.h"

#include "landed_title/landed_title.h"
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
**	@brief	Initialize the region
*/
void region::initialize()
{
	//add each subregion's provinces to this one
	for (region *subregion : this->subregions) {
		if (!subregion->is_initialized()) {
			subregion->initialize();
		}

		for (Province *province : subregion->get_provinces()) {
			this->add_province(province);
		}
	}

	//add the baronies of each of this region's provinces to it
	for (Province *province : this->get_provinces()) {
		LandedTitle *county = province->get_county();
		for (LandedTitle *barony : county->get_de_jure_vassal_titles()) {
			this->add_holding(barony);
		}
	}

	data_entry_base::initialize();
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

	for (const LandedTitle *barony : this->baronies) {
		if (barony->get_holding() != nullptr) {
			holdings.push_back(barony->get_holding());
		}
	}

	return holdings;
}

QVariantList region::get_holdings_qvariant_list() const
{
	return util::container_to_qvariant_list(this->baronies);
}

/**
**	@brief	Add a population unit to the region
*/
void region::add_population_unit(std::unique_ptr<population_unit> &&population_unit)
{
	this->population_units.push_back(std::move(population_unit));
}

}
