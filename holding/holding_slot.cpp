#include "holding/holding_slot.h"

#include "culture/culture.h"
#include "culture/culture_group.h"
#include "economy/commodity.h"
#include "holding/holding.h"
#include "holding/holding_slot_type.h"
#include "landed_title/landed_title.h"
#include "map/map.h"
#include "map/map_mode.h"
#include "map/province.h"
#include "map/province_profile.h"
#include "map/terrain_type.h"
#include "map/world.h"
#include "politics/government_type.h"
#include "politics/government_type_group.h"
#include "random.h"
#include "religion/religion.h"
#include "religion/religion_group.h"
#include "script/chance_util.h"
#include "script/modifier.h"
#include "translator.h"
#include "util/container_util.h"
#include "util/vector_util.h"

namespace metternich {

std::set<std::string> holding_slot::get_database_dependencies()
{
	return {
		//so that holding slots will be added to a province/world after the holding slots within the province's/world's definition have been added to it
		province::class_identifier,
		world::class_identifier
	};
}

holding_slot::holding_slot(const std::string &identifier) : data_entry(identifier)
{
}

holding_slot::~holding_slot()
{
}

void holding_slot::initialize()
{
	if (this->get_geocoordinate().isValid()) {
		this->set_pos(this->get_province()->get_world()->get_coordinate_posf(this->get_geocoordinate()));
	}

	if (this->province_profile != nullptr) {
		this->set_province(this->province_profile->get_province());
		this->province_profile = nullptr;
	}

	if (this->is_megalopolis() && this->get_terrain() == nullptr) {
		//set the terrain of megalopolises which have no set terrain to the one most present in its provinces
		std::map<terrain_type *, int> megalopolis_terrain_counts;
		for (metternich::province *province : this->get_megalopolis_provinces()) {
			megalopolis_terrain_counts[province->get_terrain()]++;
		}

		terrain_type *best_terrain = nullptr;
		int best_terrain_count = 0;

		for (const auto &kv_pair : megalopolis_terrain_counts) {
			terrain_type *terrain = kv_pair.first;

			const int count = kv_pair.second;
			if (count > best_terrain_count) {
				best_terrain = terrain;
				best_terrain_count = count;
			}
		}

		this->set_terrain(best_terrain);
	}

	if (this->get_holding_size() == 0) {
		this->set_holding_size(this->get_default_holding_size());
	}

	if (this->get_terrain() == nullptr) {
		if (this->get_province() != nullptr) {
			this->set_terrain(this->get_province()->get_terrain());
		} else if (!this->get_territory()->get_settlement_holding_slots().empty()) {
			//set the terrain to that of the territory's first settlement holding slot
			this->set_terrain(this->get_territory()->get_settlement_holding_slots().front()->get_terrain());
		}
	}

	if (this->is_settlement()) {
		if (this->get_available_commodities().empty()) {
			//generate an available commodity for the holding if it has none
			this->generate_available_commodity();
		}
	}

	data_entry_base::initialize();
}

void holding_slot::initialize_history()
{
	if (this->get_holding() != nullptr && !this->get_holding()->is_history_initialized()) {
		this->get_holding()->initialize_history();
	}

	data_entry_base::initialize_history();
}

void holding_slot::check() const
{
	if (this->get_province() != nullptr && this->get_world() != nullptr) {
		throw std::runtime_error("Holding slot \"" + this->get_identifier() + "\" has both a province and a world.");
	}

	if (this->get_terrain() == nullptr) {
		throw std::runtime_error("Holding slot \"" + this->get_identifier() + "\" has no terrain.");
	}

	if (this->get_territory()->get_county() == nullptr) {
		throw std::runtime_error("The territory of holding slot \"" + this->get_identifier() + "\" (\"" + this->get_territory()->get_identifier() + "\") has no county.");
	}

	if (this->get_barony() != nullptr && this->get_barony()->get_de_jure_liege_title() != this->get_territory()->get_county()) {
		throw std::runtime_error("The barony of holding slot \"" + this->get_identifier() + "\" is in county \"" + this->get_barony()->get_de_jure_liege_title()->get_identifier() + "\", but its territory belongs to county \"" + this->get_territory()->get_county()->get_identifier() + "\".");
	}
}

void holding_slot::check_history() const
{
	if (this->get_holding() != nullptr) {
		try {
			this->get_holding()->check_history();
		} catch (...) {
			std::throw_with_nested(std::runtime_error("The holding of slot \"" + this->get_identifier() + "\" is in an invalid state."));
		}
	}

	this->check();
}

void holding_slot::do_day()
{
	if (this->get_holding() != nullptr) {
		this->get_holding()->do_day();
	}
}

void holding_slot::do_month()
{
	if (this->get_holding() != nullptr) {
		this->get_holding()->do_month();
	}
}

void holding_slot::do_year()
{
}

std::string holding_slot::get_name() const
{
	if (this->get_barony() != nullptr) {
		return translator::get()->translate(this->get_barony()->get_identifier_with_aliases(), this->get_tag_suffix_list_with_fallbacks());
	}

	return translator::get()->translate(holding_slot_type_to_string(this->get_type()), this->get_tag_suffix_list_with_fallbacks()) + " Slot";
}

std::vector<std::vector<std::string>> holding_slot::get_tag_suffix_list_with_fallbacks() const
{
	std::vector<std::vector<std::string>> tag_list_with_fallbacks;

	if (this->get_barony() != nullptr) {
		if (this->get_barony()->get_government_type() != nullptr) {
			tag_list_with_fallbacks.push_back({this->get_barony()->get_government_type()->get_identifier(), government_type_group_to_string(this->get_barony()->get_government_type()->get_group())});
		}
	}

	if (this->get_territory()->get_culture() != nullptr) {
		tag_list_with_fallbacks.push_back({this->get_territory()->get_culture()->get_identifier(), this->get_territory()->get_culture()->get_culture_group()->get_identifier()});
	}

	if (this->get_territory()->get_religion()) {
		tag_list_with_fallbacks.push_back({this->get_territory()->get_religion()->get_identifier(), this->get_territory()->get_religion()->get_religion_group()->get_identifier()});
	}

	return tag_list_with_fallbacks;
}

bool holding_slot::is_settlement() const
{
	return this->get_type() == holding_slot_type::settlement;
}

void holding_slot::set_barony(landed_title *barony)
{
	if (barony == this->get_barony()) {
		return;
	}

	this->barony = barony;
	barony->set_holding_slot(this);
	emit barony_changed();
}

territory *holding_slot::get_territory() const
{
	if (this->get_province() != nullptr) {
		return this->get_province();
	} else if (this->get_world() != nullptr) {
		return this->get_world();
	}

	return nullptr;
}

void holding_slot::set_province(metternich::province *province)
{
	if (province == this->get_province()) {
		return;
	}

	if (this->get_province() != nullptr) {
		disconnect(this->get_province(), &province::active_trade_routes_changed, this, &holding_slot::active_trade_routes_changed);
	}

	this->province = province;

	connect(province, &province::active_trade_routes_changed, this, &holding_slot::active_trade_routes_changed);
}

void holding_slot::set_world(metternich::world *world)
{
	if (world == this->get_world()) {
		return;
	}

	this->world = world;
}

void holding_slot::set_terrain(metternich::terrain_type *terrain)
{
	if (terrain == this->get_terrain()) {
		return;
	}

	const metternich::terrain_type *old_terrain = this->get_terrain();
	if (old_terrain != nullptr && old_terrain->get_holding_modifier() != nullptr && this->get_holding() != nullptr) {
		old_terrain->get_holding_modifier()->remove(this->get_holding());
	}

	this->terrain = terrain;
	emit terrain_changed();

	if (terrain != nullptr && terrain->get_holding_modifier() != nullptr && this->get_holding() != nullptr) {
		terrain->get_holding_modifier()->apply(this->get_holding());
	}
}

void holding_slot::set_geocoordinate(const QGeoCoordinate &geocoordinate)
{
	if (geocoordinate == this->get_geocoordinate()) {
		return;
	}

	this->geocoordinate = geocoordinate;
}

bool holding_slot::is_territory_capital() const
{
	return this->get_territory()->get_capital_holding_slot() == this;
}

QVariantList holding_slot::get_available_commodities_qvariant_list() const
{
	return container::to_qvariant_list(this->get_available_commodities());
}

void holding_slot::set_holding(qunique_ptr<metternich::holding> &&holding)
{
	this->holding = std::move(holding);
	emit holding_changed();

	if (this->get_barony() != nullptr && this->get_holding() == nullptr) {
		this->get_barony()->set_holder(nullptr);
	}

	if (this->get_province() != nullptr && this->get_type() == holding_slot_type::trading_post && map::get()->get_mode() == map_mode::trade_zone) {
		this->get_province()->update_color_for_map_mode(map::get()->get_mode());
	}
}

void holding_slot::remove_available_commodity(commodity *commodity)
{
	vector::remove(this->available_commodities, commodity);
	emit available_commodities_changed();
}

void holding_slot::generate_available_commodity()
{
	std::map<metternich::commodity *, const chance_factor<holding_slot> *> commodity_chances;
	for (metternich::commodity *commodity : commodity::get_all()) {
		if (commodity->get_chance_factor() != nullptr) {
			commodity_chances[commodity] = commodity->get_chance_factor();
		}
	}

	metternich::commodity *commodity = calculate_chance_list_result(commodity_chances, this);
	this->add_available_commodity(commodity);
}

bool holding_slot::has_any_trade_route() const
{
	return this->get_province() != nullptr && this->get_province()->has_any_trade_route();
}

bool holding_slot::has_any_active_trade_route() const
{
	return this->get_province() != nullptr && this->get_province()->has_any_active_trade_route();
}

bool holding_slot::has_any_trade_route_land_connection() const
{
	return this->get_province() != nullptr && this->get_province()->has_any_trade_route_land_connection();
}

void holding_slot::remove_megalopolis_province(metternich::province *province)
{
	vector::remove(this->megalopolis_provinces, province);
}

void holding_slot::amalgamate_megalopolis()
{
	//update data for this holding slot based on that of its megalopolis provinces

	if (this->get_holding() == nullptr) {
		this->create_holding_from_megalopolis_provinces();

		if (this->get_holding() == nullptr) {
			return;
		}
	}

	std::map<landed_title *, int> realm_counts;

	for (metternich::province *province : this->get_megalopolis_provinces()) {
		for (metternich::holding *settlement_holding : province->get_settlement_holdings()) {
			realm_counts[settlement_holding->get_barony()->get_realm()]++;
		}
	}

	//set the owner of the megalopolis holding to the holder of the realm with most holdings in its megalopolis provinces
	landed_title *best_realm = nullptr;
	int best_count = 0;
	for (const auto &kv_pair : realm_counts) {
		landed_title *realm = kv_pair.first;
		const int count = kv_pair.second;
		if (count > best_count) {
			best_realm = realm;
			best_count = count;
		}
	}

	if (best_realm != nullptr) {
		this->get_holding()->get_barony()->set_holder(best_realm->get_holder());
	}

	if (this->get_holding()->get_population_units().empty()) {
		this->move_population_from_megalopolis_provinces();
	}
}

void holding_slot::create_holding_from_megalopolis_provinces()
{
	//create a holding for this holding slot based on the most common one for megalopolis province settlements
	std::map<holding_type *, int> holding_type_counts;

	for (metternich::province *province : this->get_megalopolis_provinces()) {
		for (metternich::holding *settlement_holding : province->get_settlement_holdings()) {
			holding_type_counts[settlement_holding->get_type()]++;
		}
	}

	holding_type *best_type = nullptr;
	int best_count = 0;
	for (const auto &kv_pair : holding_type_counts) {
		holding_type *type = kv_pair.first;
		const int count = kv_pair.second;
		if (count > best_count) {
			best_type = type;
			best_count = count;
		}
	}

	if (best_type == nullptr) {
		return;
	}

	this->get_territory()->create_holding(this, best_type);
}

void holding_slot::move_population_from_megalopolis_provinces()
{
	//move population units from megalopolis province settlement holdings to this one
	for (metternich::province *province : this->get_megalopolis_provinces()) {
		for (metternich::holding *settlement_holding : province->get_settlement_holdings()) {
			settlement_holding->move_population_units_to(this->get_holding());
		}
	}

	this->get_holding()->calculate_population_groups();
	this->get_holding()->check_overpopulation();
}

}
