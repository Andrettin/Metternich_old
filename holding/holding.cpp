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
#include "holding/building_slot.h"
#include "holding/holding_slot.h"
#include "holding/holding_slot_type.h"
#include "holding/holding_type.h"
#include "landed_title/landed_title.h"
#include "map/province.h"
#include "population/population_type.h"
#include "population/population_unit.h"
#include "random.h"
#include "religion/religion.h"
#include "religion/religion_group.h"
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
	this->change_base_population_growth(defines::get()->get_base_population_growth());
	this->set_type(type);

	if (this->get_barony() != nullptr) {
		this->set_owner(this->get_barony()->get_holder());
	}

	this->change_base_population_capacity(this->get_province()->get_population_capacity_additive_modifier());
	this->change_population_capacity_modifier(this->get_province()->get_population_capacity_modifier());
	this->change_base_population_growth(this->get_province()->get_population_growth_modifier());

	if (this->is_settlement()) {
		if (!slot->get_available_commodities().empty()) {
			metternich::commodity *commodity = slot->get_available_commodities()[random::generate(slot->get_available_commodities().size())];
			this->set_commodity(commodity);
		} else {
			throw std::runtime_error("Holding slot \"" + slot->get_identifier() + "\" has no available commodities to produce.");
		}
	}

	connect(this, &holding::type_changed, this, &holding::titled_name_changed);
	connect(this, &holding::type_changed, this, &holding::portrait_path_changed);
	connect(this, &holding::culture_changed, this, &holding::portrait_path_changed);
	connect(this, &holding::religion_changed, this, &holding::portrait_path_changed);
}

/**
**	@brief	Destructor
*/
holding::~holding()
{
	this->building_slots.clear();
}

/**
**	@brief	Initialize the holding's history
*/
void holding::initialize_history()
{
	if (this->get_owner() == nullptr && this->get_barony() == nullptr) {
		this->set_owner(this->get_province()->get_county()->get_holder());
	}

	if (this->is_settlement()) {
		if (this->get_culture() == nullptr) {
			this->set_culture(this->get_province()->get_culture());
		}

		if (this->get_religion() == nullptr) {
			this->set_religion(this->get_province()->get_religion());
		}

		for (const std::unique_ptr<population_unit> &population_unit : this->get_population_units()) {
			if (!population_unit->is_history_initialized()) {
				population_unit->initialize_history();
			}
		}

		this->remove_empty_population_units();
		this->sort_population_units();
		this->calculate_population();
		this->calculate_population_groups();
		this->check_overpopulation();
	}

	data_entry_base::initialize_history();
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
	if (this->is_settlement()) {
		this->do_population_growth();

		//use index-based for loop, as pop. units may add new ones in their do_month() function, e.g. due to mixing
		size_t pop_units_size = this->population_units.size();
		for (size_t i = 0; i < pop_units_size; ++i) {
			this->population_units[i]->do_month();
		}

		this->remove_empty_population_units();
		this->calculate_population_groups();
	}
}

/**
**	@brief	Get the holding's name
**
**	@return	The holding's name
*/
std::string holding::get_name() const
{
	if (this->get_barony() != nullptr) {
		return translator::get()->translate(this->get_barony()->get_identifier_with_aliases(), this->get_tag_suffix_list_with_fallbacks());
	}

	return this->get_province()->get_name();
}

/**
**	@brief	Get the holding's type name
**
**	@return	The holding's type name
*/
std::string holding::get_type_name() const
{
	return translator::get()->translate(this->get_type()->get_identifier_with_aliases(), this->get_tag_suffix_list_with_fallbacks());
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

std::vector<std::vector<std::string>> holding::get_tag_suffix_list_with_fallbacks() const
{
	std::vector<std::vector<std::string>> tag_list_with_fallbacks;

	const metternich::culture *culture = nullptr;
	const metternich::religion *religion = nullptr;

	if (this->is_settlement()) {
		culture = this->get_culture();
		religion = this->get_religion();
	} else {
		culture = this->get_province()->get_culture();
		religion = this->get_province()->get_religion();
	}

	tag_list_with_fallbacks.push_back({culture->get_identifier(), culture->get_culture_group()->get_identifier()});
	tag_list_with_fallbacks.push_back({religion->get_identifier(), religion->get_religion_group()->get_identifier()});

	if (this->get_province()->is_coastal()) {
		tag_list_with_fallbacks.push_back({"coastal"});
	}

	return tag_list_with_fallbacks;
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
	this->calculate_building_slots();
}

/**
**	@brief	Get whether the holding is a settlement one
**
**	@return	True if the holding is a settlement one, or false otherwise
*/
bool holding::is_settlement() const
{
	return this->get_slot()->is_settlement();
}

/**
**	@brief	Get the path to the holding's portrait
**
**	@return	The path to the portrait
*/
const std::filesystem::path &holding::get_portrait_path() const
{
	std::string base_tag = this->get_type()->get_portrait_tag();

	const std::filesystem::path &portrait_path = database::get()->get_tagged_holding_portrait_path(base_tag, this->get_tag_suffix_list_with_fallbacks());
	return portrait_path;
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
**
**	@param	population_unit	The population unit
*/
void holding::add_population_unit(std::unique_ptr<population_unit> &&population_unit)
{
	this->change_population(population_unit->get_size());
	this->population_units.push_back(std::move(population_unit));
	emit population_units_changed();
}

/**
**	@brief	Get the population unit with a given type, culture, religion and phenotype, if any exist
**
**	@param	type		The population type
**	@param	culture		The culture
**	@param	religion	The religion
**	@param	phenotype	The phenotype
**
**	@return	The population unit
*/
population_unit *holding::get_population_unit(const population_type *type, const metternich::culture *culture, const metternich::religion *religion, const phenotype *phenotype) const
{
	for (const std::unique_ptr<population_unit> &population_unit : this->get_population_units()) {
		if (population_unit->get_type() != type) {
			continue;
		}

		if (population_unit->get_culture() != culture) {
			continue;
		}

		if (population_unit->get_religion() != religion) {
			continue;
		}

		if (population_unit->get_phenotype() != phenotype) {
			continue;
		}

		return population_unit.get();
	}

	return nullptr;
}

/**
**	@brief	Change the population size of the holding's population unit with a given type, culture, religion and phenotype, creating a new one if no population unit is found with those characteristics and the change is positive
**
**	@param	type		The population type
**	@param	culture		The culture
**	@param	religion	The religion
**	@param	phenotype	The phenotype
**	@param	change		The population size change
*/
void holding::change_population_size(population_type *type, metternich::culture *culture, metternich::religion *religion, phenotype *phenotype, const int change)
{
	population_unit *population_unit = this->get_population_unit(type, culture, religion, phenotype);

	if (population_unit != nullptr) {
		population_unit->change_size(change);
	} else if (change > 0) {
		auto new_population_unit = std::make_unique<metternich::population_unit>(type);
		new_population_unit->moveToThread(QApplication::instance()->thread());
		new_population_unit->set_holding(this);
		new_population_unit->set_size(change);
		new_population_unit->set_culture(culture);
		new_population_unit->set_religion(religion);
		new_population_unit->set_phenotype(phenotype);
		this->add_population_unit(std::move(new_population_unit));
	}
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
	bool removed_pop_unit = false;
	for (size_t i = 0; i < this->population_units.size();) {
		const std::unique_ptr<population_unit> &population_unit = this->population_units[i];
		if (population_unit->get_size() == 0) {
			this->population_units.erase(this->population_units.begin() + static_cast<int>(i));
			removed_pop_unit = true;
		} else {
			++i;
		}
	}

	if (removed_pop_unit) {
		emit population_units_changed();
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
	std::unique_lock<std::shared_mutex> lock(this->population_groups_mutex);

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
**	@brief	Get the holding's building slots
**
**	@return	The building slots
*/
std::vector<building_slot *> holding::get_building_slots() const
{
	std::vector<building_slot *> building_slots;

	for (const auto &kv_pair : this->building_slots) {
		building_slots.push_back(kv_pair.second.get());
	}

	std::sort(building_slots.begin(), building_slots.end(), [](building_slot *a, building_slot *b) {
		//give priority to buildings that have already been built, so that they will be displayed first
		if (a->is_built() != b->is_built()) {
			return a->is_built();
		}

		if (a->is_buildable() != b->is_buildable()) {
			return a->is_buildable();
		}

		return a->get_building()->get_name() < b->get_building()->get_name();
	});

	return building_slots;
}

/**
**	@brief	Get the holding's building slots as a QVariantList
**
**	@return	The building slots as a QVariantList
*/
QVariantList holding::get_building_slots_qvariant_list() const
{
	return container::to_qvariant_list(this->get_building_slots());
}

/**
**	@brief	Get the holding's buildings
**
**	@return	The buildings
*/
std::set<building *> holding::get_buildings() const
{
	std::set<building *> buildings;

	for (const auto &kv_pair : this->building_slots) {
		if (kv_pair.second->is_built()) {
			buildings.insert(kv_pair.first);
		}
	}

	return buildings;
}

/**
**	@brief	Get the holding's buildings as a QVariantList
**
**	@return	The buildings as a QVariantList
*/
QVariantList holding::get_buildings_qvariant_list() const
{
	return container::to_qvariant_list(this->get_buildings());
}

bool holding::has_building(building *building) const
{
	building_slot *building_slot = this->get_building_slot(building);
	return building_slot != nullptr && building_slot->is_built();
}

void holding::add_building(building *building)
{
	building_slot *building_slot = this->get_building_slot(building);
	building_slot->set_built(true);
}

void holding::remove_building(building *building)
{
	building_slot *building_slot = this->get_building_slot(building);
	building_slot->set_built(false);
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
**	@brief	Calculate the building slots for the holding
*/
void holding::calculate_building_slots()
{
	bool changed = false;

	std::set<building *> buildings_to_remove;
	for (const auto &kv_pair : this->building_slots) {
		if (!this->get_type()->has_building(kv_pair.first)) {
			buildings_to_remove.insert(kv_pair.first);
			changed = true;
		}
	}

	for (building *building : buildings_to_remove) {
		this->building_slots.erase(building);
	}

	std::set<building *> buildings_to_add;
	for (building *building : this->get_type()->get_buildings()) {
		if (!this->building_slots.contains(building)) {
			buildings_to_add.insert(building);
			changed = true;
		}
	}

	for (building *building : buildings_to_add) {
		auto new_building_slot = std::make_unique<building_slot>(building, this);
		new_building_slot->moveToThread(QApplication::instance()->thread());

		connect(new_building_slot.get(), &building_slot::built_changed, this, &holding::buildings_changed);

		//to update the sorting of building slots
		connect(new_building_slot.get(), &building_slot::built_changed, this, &holding::building_slots_changed);

		this->building_slots[building] = std::move(new_building_slot);
	}

	if (changed) {
		emit building_slots_changed();
	}
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
	std::shared_lock<std::shared_mutex> lock(this->population_groups_mutex);

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
	std::shared_lock<std::shared_mutex> lock(this->population_groups_mutex);

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
	std::shared_lock<std::shared_mutex> lock(this->population_groups_mutex);

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
