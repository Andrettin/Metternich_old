#include "holding/holding.h"

#include "culture/culture.h"
#include "culture/culture_group.h"
#include "defines.h"
#include "economy/commodity.h"
#include "economy/employment.h"
#include "economy/employment_type.h"
#include "engine_interface.h"
#include "game/game.h"
#include "holding/building.h"
#include "holding/holding_slot.h"
#include "holding/holding_type.h"
#include "landed_title/landed_title.h"
#include "map/province.h"
#include "population/population_type.h"
#include "population/population_unit.h"
#include "random.h"
#include "religion.h"
#include "script/identifiable_modifier.h"
#include "script/modifier.h"
#include "translator.h"
#include "util/container_util.h"

#include <utility>

namespace metternich {

/**
**	@brief	Constructor
**
**	@param	slot	The holding's holding slot
**	@param	type	The holding's type (e.g. city)
*/
holding::holding(metternich::holding_slot *slot, holding_type *type) : data_entry(slot->get_identifier()), slot(slot)
{
	slot->set_holding(this);
	this->change_base_population_growth(defines::get()->get_base_population_growth());
	this->set_type(type);
	this->set_owner(this->get_barony()->get_holder());
	this->change_base_population_capacity(this->get_province()->get_population_capacity_additive_modifier());
	this->change_population_capacity_modifier(this->get_province()->get_population_capacity_modifier());
	this->change_base_population_growth(this->get_province()->get_population_growth_modifier());

	if (!slot->get_available_commodities().empty()) {
		metternich::commodity *commodity = slot->get_available_commodities()[Random::generate(slot->get_available_commodities().size())];
		this->set_commodity(commodity);
	} else {
		throw std::runtime_error("Holding slot \"" + slot->get_identifier() + "\" has no available commodities to produce.");
	}

	connect(this, &holding::type_changed, this, &holding::titled_name_changed);
}

/**
**	@brief	Destructor
*/
holding::~holding()
{
}

/**
**	@brief	Initialize the holding's history
*/
void holding::initialize_history()
{
	if (this->get_province() != nullptr) {
		if (this->get_culture() == nullptr) {
			this->set_culture(this->get_province()->get_culture());
		}

		if (this->get_religion() == nullptr) {
			this->set_religion(this->get_province()->get_religion());
		}
	}

	for (const std::unique_ptr<population_unit> &population_unit : this->get_population_units()) {
		population_unit->initialize_history();
	}

	this->remove_empty_population_units();
	this->sort_population_units();
	this->calculate_population();
	this->calculate_population_groups();
	this->check_overpopulation();
}

/**
**	@brief	Do the holding's daily actions
*/
void holding::do_day()
{
	//handle construction
	if (this->get_under_construction_building() != nullptr) {
		this->change_construction_days(-1);
		if (this->get_construction_days() <= 0) {
			this->add_building(this->get_under_construction_building());
			this->set_under_construction_building(nullptr);
		}
	}

	for (auto &kv_pair : this->get_employments()) {
		const std::unique_ptr<employment> &employment = kv_pair.second;
		employment->do_day();
	}
}

/**
**	@brief	Do the holding's monthly actions
*/
void holding::do_month()
{
	this->do_population_growth();

	for (const std::unique_ptr<population_unit> &population_unit : this->get_population_units()) {
		population_unit->do_month();
	}

	this->remove_empty_population_units();
	this->calculate_population_groups();
}

/**
**	@brief	Get the holding's name
**
**	@return	The holding's name
*/
std::string holding::get_name() const
{
	return translator::get()->translate(this->get_barony()->get_identifier(), {this->get_culture()->get_identifier(), this->get_culture()->get_culture_group()->get_identifier(), this->get_religion()->get_identifier()});
}

/**
**	@brief	Get the holding's type name
**
**	@return	The holding's type name
*/
std::string holding::get_type_name() const
{
	const metternich::culture *culture = this->get_culture();
	const metternich::religion *religion = this->get_religion();

	std::vector<std::string> suffixes;
	suffixes.push_back(culture->get_identifier());
	suffixes.push_back(culture->get_culture_group()->get_identifier());
	suffixes.push_back(religion->get_identifier());

	return translator::get()->translate(this->get_type()->get_identifier(), suffixes);
}

/**
**	@brief	Get the holding's titled name
**
**	@return	The holding's titled name
*/
std::string holding::get_titled_name() const
{
	std::string titled_name = this->get_type_name() + " of ";
	titled_name += this->get_name();
	return titled_name;
}

/**
**	@brief	Get the holding's barony
**
**	@return	The holding's barony
*/
landed_title *holding::get_barony() const
{
	return this->slot->get_barony();
}

/**
**	@brief	Set the holding's type
**
**	@param	type	The holding type
*/
void holding::set_type(holding_type *type)
{
	if (type == this->get_type()) {
		return;
	}

	if (this->get_type() != nullptr && this->get_type()->get_modifier() != nullptr) {
		this->get_type()->get_modifier()->remove(this);
	}

	this->type = type;

	if (this->get_type() != nullptr && this->get_type()->get_modifier() != nullptr) {
		this->get_type()->get_modifier()->apply(this);
	}

	emit type_changed();
}

/**
**	@brief	Get the holding's province
**
**	@return	The holding's province
*/
province *holding::get_province() const
{
	return this->slot->get_province();
}

/**
**	@brief	Add a population unit to the holding
*/
void holding::add_population_unit(std::unique_ptr<population_unit> &&population_unit)
{
	this->change_population(population_unit->get_size());
	this->population_units.push_back(std::move(population_unit));
	emit population_units_changed();
}

/**
**	@brief	Get the holding's population units as a QVariantList
**
**	@return	The population units as a QVariantList
*/
QVariantList holding::get_population_units_qvariant_list() const
{
	QVariantList list;

	for (const std::unique_ptr<population_unit> &population_unit : this->get_population_units()) {
		list.append(QVariant::fromValue(population_unit.get()));
	}

	return list;
}

/**
**	@brief	Sort the holding's population units
*/
void holding::sort_population_units()
{
	std::sort(this->population_units.begin(), this->population_units.end(), [](const std::unique_ptr<population_unit> &a, const std::unique_ptr<population_unit> &b) {
		//give priority to population units with greater size, so that they will be displayed first
		return a->get_size() > b->get_size();
	});

	emit population_units_changed();
}

/**
**	@brief	Remove population units that have size 0
*/
void holding::remove_empty_population_units()
{
	for (size_t i = 0; i < this->population_units.size();) {
		const std::unique_ptr<population_unit> &population_unit = this->population_units[i];
		if (population_unit->get_size() == 0) {
			this->population_units.erase(this->population_units.begin() + static_cast<int>(i));
		} else {
			++i;
		}
	}
}

/**
**	@brief	Set the holding's population
**
**	@param	population	The new population size for the holding
*/
void holding::set_population(const int population)
{
	if (population == this->get_population()) {
		return;
	}

	const int old_population = this->get_population();
	this->population = population;
	emit population_changed();

	this->calculate_population_growth(); //population growth depends on the current population

	//change the population count for the province as well
	const int population_change = population - old_population;
	this->get_province()->change_population(population_change);
}

/**
**	@brief	Calculate the population size for the holding
*/
void holding::calculate_population()
{
	int population = 0;
	for (const std::unique_ptr<population_unit> &population_unit : this->get_population_units()) {
		population += population_unit->get_size();
	}
	this->set_population(population);
}

/**
**	@brief	Do the holding's population growth
*/
void holding::do_population_growth()
{
	const int population_growth = this->get_population_growth();

	if (population_growth == 0) {
		return;
	}

	for (const std::unique_ptr<population_unit> &population_unit : this->get_population_units()) {
		const int population_capacity_difference = this->get_population_capacity() - this->get_population();

		int change = population_unit->get_size() * population_growth / 10000;
		if (change == 0) {
			if (population_growth != 0 && population_capacity_difference != 0) {
				//if the change is zero but population growth is non-zero, then make a change of 1
				if (population_growth > 0) {
					change = 1;
				} else {
					change = -1;
				}
			}
		} else if (change > 0 && change > population_capacity_difference) {
			change = population_capacity_difference; //don't grow the population beyond capacity
		}

		population_unit->change_size(change);
	}

	this->check_overpopulation();
}

/**
**	@brief	Check if the holding is overpopulated, and if so apply the "overpopulation" modifier
*/
void holding::check_overpopulation()
{
	//give the overpopulation modifier if the holding has become overpopulated, or remove it if it was overpopulated but isn't anymore
	const bool overpopulated = this->get_population() > this->get_population_capacity();
	if (overpopulated) {
		if (this->modifiers.find(identifiable_modifier::get_overpopulation_modifier()) == this->modifiers.end()) {
			identifiable_modifier::get_overpopulation_modifier()->apply(this);
			this->modifiers.insert(identifiable_modifier::get_overpopulation_modifier());
		}
	} else {
		if (this->modifiers.find(identifiable_modifier::get_overpopulation_modifier()) != this->modifiers.end()) {
			identifiable_modifier::get_overpopulation_modifier()->remove(this);
			this->modifiers.erase(identifiable_modifier::get_overpopulation_modifier());
		}
	}
}

/**
**	@brief	Calculate the population for each culture, religion and etc.
*/
void holding::calculate_population_groups()
{
	this->population_per_type.clear();
	this->population_per_culture.clear();
	this->population_per_religion.clear();

	for (const std::unique_ptr<population_unit> &population_unit : this->get_population_units()) {
		this->population_per_type[population_unit->get_type()] += population_unit->get_size();
		this->population_per_culture[population_unit->get_culture()] += population_unit->get_size();
		this->population_per_religion[population_unit->get_religion()] += population_unit->get_size();
	}

	emit population_groups_changed();

	//update the holding's main culture and religion

	metternich::culture *plurality_culture = nullptr;
	int plurality_culture_size = 0;

	for (const auto &kv_pair : this->population_per_culture) {
		metternich::culture *culture = kv_pair.first;
		const int culture_size = kv_pair.second;
		if (plurality_culture == nullptr || culture_size > plurality_culture_size) {
			plurality_culture = culture;
			plurality_culture_size = culture_size;
		}
	}

	metternich::religion *plurality_religion = nullptr;
	int plurality_religion_size = 0;

	for (const auto &kv_pair : this->population_per_religion) {
		metternich::religion *religion = kv_pair.first;
		const int religion_size = kv_pair.second;
		if (plurality_religion == nullptr || religion_size > plurality_religion_size) {
			plurality_religion = religion;
			plurality_religion_size = religion_size;
		}
	}

	this->set_culture(plurality_culture);
	this->set_religion(plurality_religion);
}

/**
**	@brief	Get the holding's buildings as a QVariantList
**
**	@return	The buildings as a QVariantList
*/
QVariantList holding::get_buildings_qvariant_list() const
{
	return util::container_to_qvariant_list(this->get_buildings());
}

void holding::add_building(building *building)
{
	if (this->buildings.find(building) != this->buildings.end()) {
		throw std::runtime_error("Tried to add the \"" + building->get_identifier() + "\" building to a holding that already has it.");
	}

	this->buildings.insert(building);
	this->apply_building_effects(building, 1);
	emit buildings_changed();
}

void holding::remove_building(building *building)
{
	if (this->buildings.find(building) == this->buildings.end()) {
		throw std::runtime_error("Tried to remove the \"" + building->get_identifier() + "\" building to a holding that does not have it.");
	}

	this->buildings.erase(building);
	this->apply_building_effects(building, -1);
	emit buildings_changed();
}

/**
**	@brief	Apply a building's effects to the holding
**
**	@param	building	The building
**	@param	change		The multiplier for the change: 1 to apply, -1 to remove
*/
void holding::apply_building_effects(const building *building, const int change)
{
	if (building->get_employment_type() != nullptr) {
		this->change_employment_workforce(building->get_employment_type(), building->get_workforce() * change);
	}
}

/**
**	@brief	Get the holding's available buildings (including already constructed ones)
**
**	@return	The available buildings
*/
std::vector<building *> holding::get_available_buildings() const
{
	std::vector<building *> available_buildings;

	for (building *building : this->get_type()->get_buildings()) {
		if (building->is_available_for_holding(this)) {
			available_buildings.push_back(building);
		}
	}

	std::sort(available_buildings.begin(), available_buildings.end(), [this](building *a, building *b) {
		//give priority to buildings that have already been built, so that they will be displayed first
		bool a_built = this->get_buildings().find(a) != this->get_buildings().end();
		bool b_built = this->get_buildings().find(b) != this->get_buildings().end();
		if (a_built != b_built) {
			return a_built;
		}

		return a->get_name() < b->get_name();
	});

	return available_buildings;
}

/**
**	@brief	Get the holding's available buildings as a QVariantList
**
**	@return	The available buildings as a QVariantList
*/
QVariantList holding::get_available_buildings_qvariant_list() const
{
	return util::container_to_qvariant_list(this->get_available_buildings());
}

/**
**	@brief	Set the under construction building for the holding
**
**	@param	building	The building
*/
void holding::set_under_construction_building(building *building)
{
	if (building == this->get_under_construction_building()) {
		return;
	}

	this->under_construction_building = building;
	emit under_construction_building_changed();
	if (building != nullptr) {
		this->set_construction_days(building->get_construction_days());
	}
}

/**
**	@brief	Get the employment workforce for a particular employment type
**
**	@return	The workforce
*/
int holding::get_employment_workforce(const employment_type *employment_type) const
{
	auto find_iterator = this->employments.find(employment_type);
	if (find_iterator == this->employments.end()) {
		return 0;
	}

	return find_iterator->second->get_workforce_capacity();
}

/**
**	@brief	Set the employment workforce for a particular employment type
**
**	@param	employment_type	The employment type
**	@param	workforce		The new workforce value
*/
void holding::set_employment_workforce(const employment_type *employment_type, const int workforce)
{
	if (workforce == this->get_employment_workforce(employment_type)) {
		return;
	}

	if (workforce < 0) {
		throw std::runtime_error("Tried to set a negative employment workforce for employment type \"" + employment_type->get_identifier() + "\" for a holding.");
	}

	if (workforce == 0) {
		this->employments.erase(employment_type);
	} else {
		if (this->employments.find(employment_type) == this->employments.end()) {
			this->employments[employment_type] = std::make_unique<employment>(employment_type);
		}

		this->employments[employment_type]->set_workforce_capacity(workforce);
	}
}

/**
**	@brief	Set whether the holding is selected
**
**	@param	selected	Whether the holding is being selected
**	@param	notify_engine_interface	Whether to emit a signal notifying the engine interface of the change
*/
void holding::set_selected(const bool selected, const bool notify_engine_interface)
{
	if (selected == this->is_selected()) {
		return;
	}

	if (selected) {
		if (holding::selected_holding != nullptr) {
			holding::selected_holding->set_selected(false, false);
		}
		holding::selected_holding = this;
	} else {
		holding::selected_holding = nullptr;
	}

	this->selected = selected;
	emit selected_changed();

	if (notify_engine_interface) {
		engine_interface::get()->emit selected_holding_changed();
	}
}

QVariantList holding::get_population_per_type_qvariant_list() const
{
	QVariantList population_per_type;

	for (const auto &kv_pair : this->population_per_type) {
		QVariantMap type_population;
		type_population["type"] = QVariant::fromValue(kv_pair.first);
		type_population["population"] = QVariant::fromValue(kv_pair.second);
		population_per_type.append(type_population);
	}

	return population_per_type;
}

QVariantList holding::get_population_per_culture_qvariant_list() const
{
	QVariantList population_per_culture;

	for (const auto &kv_pair : this->population_per_culture) {
		QVariantMap culture_population;
		culture_population["culture"] = QVariant::fromValue(kv_pair.first);
		culture_population["population"] = QVariant::fromValue(kv_pair.second);
		population_per_culture.append(culture_population);
	}

	return population_per_culture;
}

QVariantList holding::get_population_per_religion_qvariant_list() const
{
	QVariantList population_per_religion;

	for (const auto &kv_pair : this->population_per_religion) {
		QVariantMap religion_population;
		religion_population["religion"] = QVariant::fromValue(kv_pair.first);
		religion_population["population"] = QVariant::fromValue(kv_pair.second);
		population_per_religion.append(religion_population);
	}

	return population_per_religion;
}

void holding::order_construction(const QVariant &building_variant)
{
	QObject *building_object = qvariant_cast<QObject *>(building_variant);
	building *building = static_cast<metternich::building *>(building_object);
	game::get()->post_order([this, building]() {
		this->set_under_construction_building(building);
	});
}

}
