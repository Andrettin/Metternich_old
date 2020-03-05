#include "map/region.h"

#include "database/database.h"
#include "holding/holding_slot.h"
#include "landed_title/landed_title.h"
#include "map/province.h"
#include "map/world.h"
#include "population/population_unit.h"
#include "species/wildlife_unit.h"
#include "technology/technology.h"
#include "util/container_util.h"

namespace metternich {

std::set<std::string> region::get_database_dependencies()
{
	return {
		//so that when regions are processed provinces and worlds already have their holding slots set
		province::class_identifier,
		holding_slot::class_identifier,
		world::class_identifier
	};
}

region::region(const std::string &identifier) : data_entry(identifier)
{
}

region::~region()
{
}

void region::initialize_history()
{
	this->population_units.clear();
	this->wildlife_units.clear();

	for (technology *technology : this->technologies) {
		for (territory *territory : this->territories) {
			territory->add_technology(technology);
		}
	}
	this->technologies.clear();

	data_entry_base::initialize_history();
}

void region::add_territory(territory *territory)
{
	this->territories.insert(territory);
	territory->add_region(this);

	//add the holdings belonging to the territory to the region
	for (holding_slot *holding_slot : territory->get_settlement_holding_slots()) {
		this->add_holding(holding_slot);
	}
}

void region::remove_territory(territory *territory)
{
	this->territories.erase(territory);
	territory->remove_region(this);

	//remove the holdings belonging to the territory from the region
	for (holding_slot *holding_slot : territory->get_settlement_holding_slots()) {
		this->remove_holding(holding_slot);
	}
}

QVariantList region::get_provinces_qvariant_list() const
{
	return container::to_qvariant_list(this->get_provinces());
}

void region::add_province(province *province)
{
	this->provinces.insert(province);
	this->add_territory(province);

	for (region *superregion : this->superregions) {
		superregion->add_province(province);
	}
}

void region::remove_province(province *province)
{
	this->provinces.erase(province);
	this->remove_territory(province);

	for (region *superregion : this->superregions) {
		superregion->remove_province(province);
	}
}

QVariantList region::get_worlds_qvariant_list() const
{
	return container::to_qvariant_list(this->get_worlds());
}

void region::add_world(world *world)
{
	this->worlds.insert(world);
	this->add_territory(world);

	for (region *superregion : this->superregions) {
		superregion->add_world(world);
	}
}

void region::remove_world(world *world)
{
	this->worlds.erase(world);
	this->remove_territory(world);

	for (region *superregion : this->superregions) {
		superregion->remove_world(world);
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

QVariantList region::get_technologies_qvariant_list() const
{
	return container::to_qvariant_list(this->technologies);
}

void region::add_population_unit(qunique_ptr<population_unit> &&population_unit)
{
	this->population_units.push_back(std::move(population_unit));
}

void region::add_wildlife_unit(qunique_ptr<wildlife_unit> &&wildlife_unit)
{
	this->wildlife_units.push_back(std::move(wildlife_unit));
}

}
