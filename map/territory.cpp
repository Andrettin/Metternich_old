#include "map/territory.h"

#include "culture/culture.h"
#include "culture/culture_group.h"
#include "defines.h"
#include "holding/building.h"
#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "holding/holding_slot_type.h"
#include "holding/holding_type.h"
#include "landed_title/landed_title.h"
#include "politics/government_type.h"
#include "politics/government_type_group.h"
#include "population/population_type.h"
#include "population/population_unit.h"
#include "religion/religion.h"
#include "religion/religion_group.h"
#include "translator.h"
#include "util/container_util.h"
#include "util/vector_util.h"

namespace metternich {

territory::territory(const std::string &identifier) : data_entry(identifier)
{
}

territory::~territory()
{
}

void territory::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag.substr(0, 2) == holding_slot::prefix) {
		holding_slot *holding_slot = nullptr;
		if (scope.get_operator() == gsml_operator::assignment) {
			holding_slot = holding_slot::add(tag);
		} else if (scope.get_operator() == gsml_operator::addition) {
			holding_slot = this->get_holding_slot(tag);
		} else {
			throw std::runtime_error("Invalid operator for scope (\"" + tag + "\").");
		}

		database::process_gsml_data(holding_slot, scope);

		if (scope.get_operator() == gsml_operator::assignment) {
			this->add_holding_slot(holding_slot);
		}
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

void territory::process_gsml_dated_property(const gsml_property &property, const QDateTime &date)
{
	Q_UNUSED(date)

	if (property.get_key().substr(0, 2) == holding_slot::prefix || is_holding_slot_type_string(property.get_key())) {
		//a property related to one of the territory's holdings
		holding_slot *holding_slot = this->get_holding_slot(property.get_key());

		if (holding_slot == nullptr) {
			throw std::runtime_error("Territory \"" + this->get_identifier() + "\" has no holding slot for string \"" + property.get_key() + "\".");
		}

		holding *holding = holding_slot->get_holding();

		if (property.get_operator() == gsml_operator::assignment) {
			//the assignment operator sets the holding's type (creating the holding if it doesn't exist)
			landed_title *title = landed_title::try_get(property.get_value());
			if (title != nullptr) {
				if (title->get_holder() == nullptr) {
					throw std::runtime_error("Tried to set the owner for holding of slot \"" + holding_slot->get_identifier() + "\" to the holder of landed title \"" + title->get_identifier() + "\", but the latter has no holder.");
				}

				holding->set_owner(title->get_holder());
				return;
			}

			holding_type *holding_type = holding_type::get(property.get_value());
			if (holding != nullptr) {
				if (holding_type != nullptr) {
					holding->set_type(holding_type);
				} else {
					this->destroy_holding(holding_slot);
				}
			} else {
				if (holding_type != nullptr) {
					this->create_holding(holding_slot, holding_type);
				}
			}
		} else if (property.get_operator() == gsml_operator::addition || property.get_operator() == gsml_operator::subtraction) {
			if (holding == nullptr) {
				throw std::runtime_error("Tried to add or remove a building for an unbuilt holding (holding slot \"" + holding_slot->get_identifier() + "\").");
			}

			//the addition/subtraction operators add/remove buildings to/from the holding
			building *building = building::get(property.get_value());
			if (property.get_operator() == gsml_operator::addition) {
				holding->add_building(building);
			} else if (property.get_operator() == gsml_operator::subtraction) {
				holding->remove_building(building);
			}
		}
	} else {
		data_entry_base::process_gsml_property(property);
	}
}

void territory::process_gsml_dated_scope(const gsml_data &scope, const QDateTime &date)
{
	const std::string &tag = scope.get_tag();

	if (tag.substr(0, 2) == holding_slot::prefix || is_holding_slot_type_string(tag)) {
		//a change to the data of one of the territory's holdings

		holding_slot *holding_slot = this->get_holding_slot(tag);
		holding *holding = holding_slot->get_holding();
		if (holding != nullptr) {
			scope.for_each_property([&](const gsml_property &property) {
				holding->process_gsml_dated_property(property, date);
			});
		} else {
			throw std::runtime_error("Territory \"" + this->get_identifier() + "\" has no constructed holding for holding slot \"" + holding_slot->get_identifier() + "\", while having history to change the holding's data.");
		}
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

void territory::initialize()
{
	if (this->get_county() != nullptr) {
		connect(this->get_county(), &landed_title::holder_changed, this, &territory::owner_changed);

		//create a fort holding slot for this territory if none exists
		if (this->get_fort_holding_slot() == nullptr) {
			std::string holding_slot_identifier = holding_slot::prefix + this->get_identifier_without_prefix() + "_fort";
			holding_slot *holding_slot = holding_slot::add(holding_slot_identifier);
			holding_slot->set_type(holding_slot_type::fort);
			this->add_holding_slot(holding_slot);
		}

		//create a university holding slot for this territory if none exists
		if (this->get_university_holding_slot() == nullptr) {
			std::string holding_slot_identifier = holding_slot::prefix + this->get_identifier_without_prefix() + "_university";
			holding_slot *holding_slot = holding_slot::add(holding_slot_identifier);
			holding_slot->set_type(holding_slot_type::university);
			this->add_holding_slot(holding_slot);
		}

		//create a hospital holding slot for this territory if none exists
		if (this->get_hospital_holding_slot() == nullptr) {
			std::string holding_slot_identifier = holding_slot::prefix + this->get_identifier_without_prefix() + "_hospital";
			holding_slot *holding_slot = holding_slot::add(holding_slot_identifier);
			holding_slot->set_type(holding_slot_type::hospital);
			this->add_holding_slot(holding_slot);
		}

		//create a factory holding slot for this territory if none exists
		if (this->get_factory_holding_slot() == nullptr) {
			std::string holding_slot_identifier = holding_slot::prefix + this->get_identifier_without_prefix() + "_factory";
			holding_slot *holding_slot = holding_slot::add(holding_slot_identifier);
			holding_slot->set_type(holding_slot_type::factory);
			this->add_holding_slot(holding_slot);
		}

		//create a trading post holding slot for this territory if none exists
		if (this->get_trading_post_holding_slot() == nullptr) {
			this->create_trading_post_holding_slot();
		}
	}

	data_entry_base::initialize();
}

void territory::initialize_history()
{
	this->population_units.clear();

	//ensure the territory's settlement holding slots have been initialized, so that its culture and religion will be calculated correctly
	for (holding_slot *settlement_holding_slot : this->get_settlement_holding_slots()) {
		if (!settlement_holding_slot->is_history_initialized()) {
			settlement_holding_slot->initialize_history();
		}
	}

	if (this->get_county() != nullptr) {
		this->calculate_population();
		this->calculate_population_groups();

		if (!this->can_have_trading_post()) {
			//destroy the created trading post holding slot if the territory can't currently have a trading post
			this->destroy_trading_post_holding_slot();
		}
	}

	data_entry_base::initialize_history();
}

void territory::check() const
{
	if (this->get_county() != nullptr) {
		if (this->get_settlement_holding_slots().empty()) {
			throw std::runtime_error("Territory \"" + this->get_identifier() + "\" has a county (not being a wasteland or water zone), but has no settlement holding slots.");
		}
	}
}

void territory::check_history() const
{
	if (this->get_county() != nullptr && !this->get_settlement_holdings().empty()) {
		if (this->get_culture() == nullptr) {
			throw std::runtime_error("Territory \"" + this->get_identifier() + "\" has no culture.");
		}

		if (this->get_religion() == nullptr) {
			throw std::runtime_error("Territory \"" + this->get_identifier() + "\" has no religion.");
		}
	}

	if (this->get_capital_holding_slot() != nullptr && this->get_capital_holding_slot()->get_territory() != this) {
		throw std::runtime_error("Territory \"" + this->get_identifier() + "\"'s capital holding slot (\"" + this->get_capital_holding_slot()->get_barony()->get_identifier() + "\") belongs to another territory (\"" + this->get_capital_holding_slot()->get_territory()->get_identifier() + "\").");
	}

	this->check();
}

void territory::do_month()
{
	this->calculate_population_groups();
}

std::string territory::get_name() const
{
	if (this->get_county() != nullptr) {
		return translator::get()->translate(this->get_county()->get_identifier_with_aliases(), this->get_tag_suffix_list_with_fallbacks());
	}

	return translator::get()->translate(this->get_identifier_with_aliases()); //territory without a county; sea zone, river, lake, wasteland or uninhabitable world
}

std::vector<std::vector<std::string>> territory::get_tag_suffix_list_with_fallbacks() const
{
	std::vector<std::vector<std::string>> tag_list_with_fallbacks;

	if (this->get_county() != nullptr) {
		if (this->get_county()->get_government_type() != nullptr) {
			tag_list_with_fallbacks.push_back({this->get_county()->get_government_type()->get_identifier(), government_type_group_to_string(this->get_county()->get_government_type()->get_group())});
		}
	}

	if (this->get_culture() != nullptr) {
		tag_list_with_fallbacks.push_back({this->get_culture()->get_identifier(), this->get_culture()->get_culture_group()->get_identifier()});
	}

	if (this->get_religion() != nullptr) {
		tag_list_with_fallbacks.push_back({this->get_religion()->get_identifier(), this->get_religion()->get_religion_group()->get_identifier()});
	}

	return tag_list_with_fallbacks;
}

void territory::set_county(landed_title *county)
{
	if (county == this->get_county()) {
		return;
	}

	this->county = county;
	emit county_changed();
}

landed_title *territory::get_duchy() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->get_duchy();
	}

	return nullptr;
}

landed_title *territory::get_de_jure_duchy() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->get_de_jure_duchy();
	}

	return nullptr;
}

landed_title *territory::get_kingdom() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->get_kingdom();
	}

	return nullptr;
}

landed_title *territory::get_de_jure_kingdom() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->get_de_jure_kingdom();
	}

	return nullptr;
}

landed_title *territory::get_empire() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->get_empire();
	}

	return nullptr;
}

landed_title *territory::get_de_jure_empire() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->get_de_jure_empire();
	}

	return nullptr;
}

character *territory::get_owner() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->get_holder();
	}

	return nullptr;
}


void territory::set_culture(metternich::culture *culture)
{
	if (culture == this->get_culture()) {
		return;
	}

	if (culture == nullptr && this->get_owner() != nullptr) {
		throw std::runtime_error("Tried to set the culture of territory \"" + this->get_identifier() + "\" to null, despite it having an owner.");
	}

	this->culture = culture;
	emit culture_changed();
}

void territory::set_religion(metternich::religion *religion)
{
	if (religion == this->get_religion()) {
		return;
	}

	if (religion == nullptr && this->get_owner() != nullptr) {
		throw std::runtime_error("Tried to set the religion of territory \"" + this->get_identifier() + "\" to null, despite it having an owner.");
	}

	this->religion = religion;
	emit religion_changed();
}

holding_slot *territory::get_holding_slot(const std::string &holding_slot_str) const
{
	if (holding_slot_str.substr(0, 2) == holding_slot::prefix) {
		holding_slot *holding_slot = holding_slot::get(holding_slot_str);

		if (holding_slot->get_territory() != this) {
			throw std::runtime_error("Tried to get holding slot \"" + holding_slot->get_identifier() + "\" for territory \"" + this->get_identifier() + "\", but the holding slot belongs to another territory.");
		}

		return holding_slot;
	} else {
		holding_slot_type slot_type = string_to_holding_slot_type(holding_slot_str);
		switch (slot_type) {
			case holding_slot_type::fort:
				return this->get_fort_holding_slot();
			case holding_slot_type::university:
				return this->get_university_holding_slot();
			case holding_slot_type::hospital:
				return this->get_hospital_holding_slot();
			case holding_slot_type::trading_post:
				if (this->get_trading_post_holding_slot() == nullptr) {
					throw std::runtime_error("Tried to get the trading post holding slot of territory \"" + this->get_identifier() + "\", which has no such slot.");
				}

				return this->get_trading_post_holding_slot();
			case holding_slot_type::factory:
				return this->get_factory_holding_slot();
			default:
				break;
		}
	}

	throw std::runtime_error("\"" + holding_slot_str + "\" is not a valid holding slot string for territory data.");
}

void territory::add_holding_slot(holding_slot *holding_slot)
{
	switch (holding_slot->get_type()) {
		case holding_slot_type::settlement:
			this->settlement_holding_slots.push_back(holding_slot);
			emit settlement_holding_slots_changed();
			if (this->get_capital_holding_slot() == nullptr) {
				//set the first settlement holding slot as the capital if none has been set
				this->set_capital_holding_slot(holding_slot);
			}
			break;
		case holding_slot_type::palace:
			this->palace_holding_slots.push_back(holding_slot);
			break;
		case holding_slot_type::fort:
			this->fort_holding_slot = holding_slot;
			break;
		case holding_slot_type::university:
			this->university_holding_slot = holding_slot;
			break;
		case holding_slot_type::hospital:
			this->hospital_holding_slot = holding_slot;
			break;
		case holding_slot_type::trading_post:
			this->trading_post_holding_slot = holding_slot;
			break;
		case holding_slot_type::factory:
			this->factory_holding_slot = holding_slot;
			break;
		default:
			throw std::runtime_error("Holding slot \"" + holding_slot->get_identifier() + "\" has an invalid type (" + std::to_string(static_cast<int>(holding_slot->get_type())) + "), but is being added to territory \"" + this->get_identifier() + "\".");
	}
}

void territory::create_holding(holding_slot *holding_slot, holding_type *type)
{
	auto new_holding = make_qunique<holding>(holding_slot, type);
	new_holding->moveToThread(QApplication::instance()->thread());
	holding_slot->set_holding(std::move(new_holding));
	switch (holding_slot->get_type()) {
		case holding_slot_type::settlement:
			this->settlement_holdings.push_back(holding_slot->get_holding());
			emit settlement_holdings_changed();

			if (this->get_capital_holding() == nullptr) {
				this->set_capital_holding(holding_slot->get_holding());
			}

			break;
		case holding_slot_type::fort:
		case holding_slot_type::hospital:
		case holding_slot_type::university:
		case holding_slot_type::trading_post:
		case holding_slot_type::factory:
			holding_slot->get_holding()->set_owner(this->get_owner());
			break;
		default:
			break;
	}
}

void territory::destroy_holding(holding_slot *holding_slot)
{
	holding *holding = holding_slot->get_holding();

	if (holding_slot->get_type() == holding_slot_type::settlement) {
		vector::remove(this->settlement_holdings, holding);

		if (holding == this->get_capital_holding()) {
			//if the capital holding is being destroyed, set the next holding as the capital, if any exists, or otherwise set the capital holding to null
			if (!this->settlement_holdings.empty()) {
				this->set_capital_holding(this->settlement_holdings.front());
			} else {
				this->set_capital_holding(nullptr);
				//if the last settlement holding was destroyed in the territory, make it an empty one
				this->destroy_special_holdings(); //destroy all remaining holdings
				this->get_county()->set_holder(nullptr);
			}
		}

		emit settlement_holdings_changed();
	}

	holding_slot->set_holding(nullptr);
}

void territory::destroy_special_holdings()
{
	//destroy non-settlement holdings

	for (holding_slot *holding_slot : this->get_palace_holding_slots()) {
		holding *holding = holding_slot->get_holding();

		if (holding != nullptr) {
			this->destroy_holding(holding_slot);
		}
	}

	if (this->get_fort_holding_slot()->get_holding() != nullptr) {
		this->destroy_holding(this->get_fort_holding_slot());
	}
	if (this->get_university_holding_slot()->get_holding() != nullptr) {
		this->destroy_holding(this->get_university_holding_slot());
	}
	if (this->get_hospital_holding_slot()->get_holding() != nullptr) {
		this->destroy_holding(this->get_hospital_holding_slot());
	}
	if (this->get_trading_post_holding() != nullptr) {
		this->destroy_holding(this->get_trading_post_holding_slot());
	}
	if (this->get_factory_holding_slot()->get_holding() != nullptr) {
		this->destroy_holding(this->get_factory_holding_slot());
	}
}

QVariantList territory::get_settlement_holding_slots_qvariant_list() const
{
	return container::to_qvariant_list(this->get_settlement_holding_slots());
}

QVariantList territory::get_settlement_holdings_qvariant_list() const
{
	return container::to_qvariant_list(this->get_settlement_holdings());
}

void territory::destroy_settlement_holdings()
{
	std::vector<holding *> settlement_holdings = this->get_settlement_holdings();

	for (holding *holding : settlement_holdings) {
		this->destroy_holding(holding->get_slot());
	}
}

QVariantList territory::get_palace_holding_slots_qvariant_list() const
{
	return container::to_qvariant_list(this->get_palace_holding_slots());
}

std::string territory::get_trading_post_holding_slot_identifier() const
{
	return holding_slot::prefix + this->get_identifier_without_prefix() + "_trading_post";
}

void territory::create_trading_post_holding_slot()
{
	std::string holding_slot_identifier = this->get_trading_post_holding_slot_identifier();
	holding_slot *holding_slot = holding_slot::add(holding_slot_identifier);
	holding_slot->set_type(holding_slot_type::trading_post);
	this->add_holding_slot(holding_slot);
	emit trading_post_holding_slot_changed();
}

void territory::destroy_trading_post_holding_slot()
{
	this->get_trading_post_holding_slot()->set_holding(nullptr);
	std::string holding_slot_identifier = this->get_trading_post_holding_slot_identifier();
	holding_slot::remove(holding_slot_identifier);
	this->trading_post_holding_slot = nullptr;
	emit trading_post_holding_slot_changed();
}

holding *territory::get_trading_post_holding() const
{
	if (this->get_trading_post_holding_slot() != nullptr) {
		return this->get_trading_post_holding_slot()->get_holding();
	}

	return nullptr;
}

void territory::set_capital_holding_slot(holding_slot *holding_slot)
{
	if (holding_slot == this->get_capital_holding_slot()) {
		return;
	}

	if (holding_slot != nullptr) {
		if (holding_slot->get_territory() != this) {
			throw std::runtime_error("Tried to set holding \"" + holding_slot->get_identifier() + "\" as the capital holding of territory \"" + this->get_identifier() + "\", but it belongs to another territory.");
		}

		if (holding_slot->get_type() != holding_slot_type::settlement) {
			throw std::runtime_error("Tried to set holding \"" + holding_slot->get_identifier() + "\" as the capital holding of territory \"" + this->get_identifier() + "\", but it is not a settlement holding.");
		}
	}

	this->capital_holding_slot = holding_slot;
	emit capital_holding_slot_changed();
}

holding *territory::get_capital_holding() const
{
	if (this->get_capital_holding_slot() != nullptr) {
		return this->get_capital_holding_slot()->get_holding();
	}

	return nullptr;
}

void territory::set_capital_holding(holding *holding)
{
	if (holding == this->get_capital_holding()) {
		return;
	}

	if (holding != nullptr) {
		this->set_capital_holding_slot(holding->get_slot());
	} else if (!this->get_settlement_holding_slots().empty()) {
		this->set_capital_holding_slot(this->get_settlement_holding_slots().front());
	} else {
		this->set_capital_holding_slot(nullptr);
	}
}

void territory::set_population(const int population)
{
	if (population == this->get_population()) {
		return;
	}

	this->population = population;
	emit population_changed();
}

void territory::calculate_population()
{
	int population = 0;
	for (const holding *holding : this->get_settlement_holdings()) {
		population += holding->get_population();
	}
	this->set_population(population);
}

void territory::set_population_capacity_additive_modifier(const int population_capacity_modifier)
{
	if (population_capacity_modifier == this->get_population_capacity_additive_modifier()) {
		return;
	}

	for (holding *holding : this->get_settlement_holdings()) {
		holding->change_base_population_capacity(-this->get_population_capacity_additive_modifier());
	}

	this->population_capacity_additive_modifier = population_capacity_modifier;

	for (holding *holding : this->get_settlement_holdings()) {
		holding->change_base_population_capacity(this->get_population_capacity_additive_modifier());
	}
}

void territory::set_population_capacity_modifier(const int population_capacity_modifier)
{
	if (population_capacity_modifier == this->get_population_capacity_modifier()) {
		return;
	}

	for (holding *holding : this->get_settlement_holdings()) {
		holding->change_population_capacity_modifier(-this->get_population_capacity_modifier());
	}

	this->population_capacity_modifier = population_capacity_modifier;

	for (holding *holding : this->get_settlement_holdings()) {
		holding->change_population_capacity_modifier(this->get_population_capacity_modifier());
	}
}

void territory::set_population_growth_modifier(const int population_growth_modifier)
{
	if (population_growth_modifier == this->get_population_growth_modifier()) {
		return;
	}

	for (holding *holding : this->get_settlement_holdings()) {
		holding->change_base_population_growth(-this->get_population_growth_modifier());
	}

	this->population_growth_modifier = population_growth_modifier;

	for (holding *holding : this->get_settlement_holdings()) {
		holding->change_base_population_growth(this->get_population_growth_modifier());
	}
}

void territory::calculate_population_groups()
{
	std::unique_lock<std::shared_mutex> lock(this->population_groups_mutex);

	this->population_per_type.clear();
	this->population_per_culture.clear();
	this->population_per_religion.clear();

	for (holding *holding : this->get_settlement_holdings()) {
		for (const auto &kv_pair : holding->get_population_per_type()) {
			this->population_per_type[kv_pair.first] += kv_pair.second;
		}
		for (const auto &kv_pair : holding->get_population_per_culture()) {
			this->population_per_culture[kv_pair.first] += kv_pair.second;
		}
		for (const auto &kv_pair : holding->get_population_per_religion()) {
			this->population_per_religion[kv_pair.first] += kv_pair.second;
		}
	}

	emit population_groups_changed();

	//update the territory's main culture and religion

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

QVariantList territory::get_population_per_type_qvariant_list() const
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

QVariantList territory::get_population_per_culture_qvariant_list() const
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

QVariantList territory::get_population_per_religion_qvariant_list() const
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

bool territory::is_selectable() const
{
	return this->get_county() != nullptr;
}

void territory::add_population_unit(qunique_ptr<population_unit> &&population_unit)
{
	this->population_units.push_back(std::move(population_unit));
}

}
