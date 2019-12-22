#include "map/region.h"

#include "database/database.h"
#include "holding/holding_slot.h"
#include "landed_title/landed_title.h"
#include "map/province.h"
#include "population/population_unit.h"
#include "species/wildlife_unit.h"
#include "util/container_util.h"

namespace metternich {

/**
**	@brief	Get the string identifiers of the classes on which this one depends for loading its database
**
**	@return	The class identifier string list
*/
std::set<std::string> region::get_database_dependencies()
{
	return {
		//so that when regions are processed provinces already have their holding slots set
		province::class_identifier,
		holding_slot::class_identifier
	};
}

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
	this->wildlife_units.clear();

	data_entry_base::initialize_history();
}

QVariantList region::get_provinces_qvariant_list() const
{
	return container::to_qvariant_list(this->get_provinces());
}

void region::add_province(province *province)
{
	this->provinces.insert(province);
	province->add_region(this);

	//add the holdings belonging to the province to the region
	for (holding_slot *holding_slot : province->get_settlement_holding_slots()) {
		this->add_holding(holding_slot);
	}

	for (region *superregion : this->superregions) {
		superregion->add_province(province);
	}
}

void region::remove_province(province *province)
{
	this->provinces.erase(province);
	province->remove_region(this);

	//remove the holdings belonging to the province from the region
	for (holding_slot *holding_slot : province->get_settlement_holding_slots()) {
		this->remove_holding(holding_slot);
	}

	for (region *superregion : this->superregions) {
		superregion->remove_province(province);
	}
}

QVariantList region::get_subregions_qvariant_list() const
{
	return container::to_qvariant_list(this->subregions);
}

std::vector<holding *> region::get_holdings() const
{
	std::vector<holding *> holdings;

	for (const holding_slot *holding_slot : this->holding_slots) {
		if (holding_slot->get_holding() != nullptr) {
			holdings.push_back(holding_slot->get_holding());
		}
	}

	return holdings;
}

QVariantList region::get_holdings_qvariant_list() const
{
	return container::to_qvariant_list(this->holding_slots);
}

/**
**	@brief	Add a population unit to the region
*/
void region::add_population_unit(std::unique_ptr<population_unit> &&population_unit)
{
	this->population_units.push_back(std::move(population_unit));
}

void region::add_wildlife_unit(std::unique_ptr<wildlife_unit> &&wildlife_unit)
{
	this->wildlife_units.push_back(std::move(wildlife_unit));
}

}
