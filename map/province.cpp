#include "map/province.h"

#include "culture/culture.h"
#include "culture/culture_group.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "defines.h"
#include "economy/trade_node.h"
#include "economy/trade_route.h"
#include "engine_interface.h"
#include "game/game.h"
#include "history/history.h"
#include "holding/building.h"
#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "holding/holding_slot_type.h"
#include "holding/holding_type.h"
#include "landed_title/landed_title.h"
#include "landed_title/landed_title_tier.h"
#include "map/map.h"
#include "map/map_mode.h"
#include "map/pathfinder.h"
#include "map/region.h"
#include "map/terrain_type.h"
#include "map/world.h"
#include "politics/government_type.h"
#include "politics/government_type_group.h"
#include "population/population_type.h"
#include "population/population_unit.h"
#include "religion/religion.h"
#include "religion/religion_group.h"
#include "script/modifier.h"
#include "species/wildlife_unit.h"
#include "technology/technology.h"
#include "translator.h"
#include "util/container_util.h"
#include "util/location_util.h"
#include "util/point_util.h"

#include <QApplication>
#include <QGeoCircle>
#include <QtLocation/private/qgeojson_p.h>

namespace metternich {

std::set<std::string> province::get_database_dependencies()
{
	return {
		//so that baronies will be ensured to exist when provinces (and thus holding slots) are processed
		landed_title::class_identifier,
		//so that the effects of the set_terrain() function can occur properly
		terrain_type::class_identifier
	};
}

province *province::get_by_rgb(const QRgb &rgb, const bool should_find)
{
	if (rgb == province::empty_rgb) {
		return nullptr;
	}

	typename std::map<QRgb, province *>::const_iterator find_iterator = province::instances_by_rgb.find(rgb);

	if (find_iterator != province::instances_by_rgb.end()) {
		return find_iterator->second;
	}

	if (should_find) {
		QColor color(rgb);
		throw std::runtime_error("No province found for RGB value: " + std::to_string(color.red()) + "/" + std::to_string(color.green()) + "/" + std::to_string(color.blue()) + ".");
	}

	return nullptr;
}

province *province::add(const std::string &identifier)
{
	if (identifier.substr(0, 2) != province::prefix) {
		throw std::runtime_error("Invalid identifier for new province: \"" + identifier + "\". Province identifiers must begin with \"" + province::prefix + "\".");
	}

	return data_type<province>::add(identifier);
}

province::province(const std::string &identifier) : data_entry(identifier)
{
	connect(this, &province::culture_changed, this, &identifiable_data_entry_base::name_changed);
	connect(this, &province::religion_changed, this, &identifiable_data_entry_base::name_changed);
	connect(game::get(), &game::running_changed, this, &province::update_image);
}

province::~province()
{
}

void province::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "color") {
		this->color = scope.to_color();

		if (province::instances_by_rgb.find(this->color.rgb()) != province::instances_by_rgb.end()) {
			throw std::runtime_error("The color set for province \"" + this->get_identifier() + "\" is already used by province \"" + province::instances_by_rgb.find(this->color.rgb())->second->get_identifier() + "\"");
		}

		province::instances_by_rgb[this->color.rgb()] = this;
	} else if (tag == "geopolygons") {
		for (const gsml_data &polygon_data : scope.get_children()) {
			this->geopolygons.push_back(polygon_data.to_geopolygon());
		}
	} else if (tag == "geopaths") {
		for (const gsml_data &path_data : scope.get_children()) {
			QGeoPath geopath = path_data.to_geopath();
			if (this->get_terrain() != nullptr) {
				geopath.setWidth(this->get_terrain()->get_path_width());
			}
			this->geopaths.push_back(std::move(geopath));
		}
	} else if (tag == "border_provinces") {
		for (const std::string &border_province_identifier : scope.get_values()) {
			province *border_province = province::get(border_province_identifier);
			this->border_provinces.insert(border_province);
		}
	} else if (tag == "path_pos_list") {
		for (const gsml_data &pos_data : scope.get_children()) {
			this->add_path_pos(pos_data.to_point());
		}
	} else if (tag.substr(0, 2) == holding_slot::prefix) {
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
			holding_slot->set_province(this);
		}
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

void province::process_gsml_dated_property(const gsml_property &property, const QDateTime &date)
{
	Q_UNUSED(date)

	if (property.get_key().substr(0, 2) == holding_slot::prefix || is_holding_slot_type_string(property.get_key())) {
		//a property related to one of the province's holdings
		holding_slot *holding_slot = this->get_holding_slot(property.get_key());
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

void province::process_gsml_dated_scope(const gsml_data &scope, const QDateTime &date)
{
	const std::string &tag = scope.get_tag();

	if (tag.substr(0, 2) == holding_slot::prefix || is_holding_slot_type_string(tag)) {
		//a change to the data of one of the province's holdings

		holding_slot *holding_slot = this->get_holding_slot(tag);
		holding *holding = holding_slot->get_holding();
		if (holding != nullptr) {
			for (const gsml_property &property : scope.get_properties()) {
				holding->process_gsml_dated_property(property, date);
			}
		} else {
			throw std::runtime_error("Province \"" + this->get_identifier() + "\" has no constructed holding for holding slot \"" + holding_slot->get_identifier() + "\", while having history to change the holding's data.");
		}
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

void province::initialize()
{
	if (this->get_county() != nullptr) {
		connect(this->get_county(), &landed_title::holder_changed, this, &province::owner_changed);

		//create a fort holding slot for this province if none exists
		if (this->get_fort_holding_slot() == nullptr) {
			std::string holding_slot_identifier = holding_slot::prefix + this->get_identifier_without_prefix() + "_fort";
			holding_slot *holding_slot = holding_slot::add(holding_slot_identifier);
			holding_slot->set_type(holding_slot_type::fort);
			holding_slot->set_province(this);
		}

		//create a university holding slot for this province if none exists
		if (this->get_university_holding_slot() == nullptr) {
			std::string holding_slot_identifier = holding_slot::prefix + this->get_identifier_without_prefix() + "_university";
			holding_slot *holding_slot = holding_slot::add(holding_slot_identifier);
			holding_slot->set_type(holding_slot_type::university);
			holding_slot->set_province(this);
		}

		//create a hospital holding slot for this province if none exists
		if (this->get_hospital_holding_slot() == nullptr) {
			std::string holding_slot_identifier = holding_slot::prefix + this->get_identifier_without_prefix() + "_hospital";
			holding_slot *holding_slot = holding_slot::add(holding_slot_identifier);
			holding_slot->set_type(holding_slot_type::hospital);
			holding_slot->set_province(this);
		}

		//create a factory holding slot for this province if none exists
		if (this->get_factory_holding_slot() == nullptr) {
			std::string holding_slot_identifier = holding_slot::prefix + this->get_identifier_without_prefix() + "_factory";
			holding_slot *holding_slot = holding_slot::add(holding_slot_identifier);
			holding_slot->set_type(holding_slot_type::factory);
			holding_slot->set_province(this);
		}

		//create a trading post holding slot for this province if none exists
		if (this->get_trading_post_holding_slot() == nullptr) {
			this->create_trading_post_holding_slot();
		}
	}

	if (this->get_world()->get_surface_area() != 0 && this->get_area() == 0) {
		this->area = this->get_world()->get_area_per_pixel() * this->pixel_count;
	}

	data_entry_base::initialize();
}

void province::initialize_history()
{
	this->population_units.clear();

	if (this->get_capital_holding_slot() == nullptr && !this->get_settlement_holding_slots().empty()) {
		//set the first settlement holding slot as the capital if none has been set
		this->set_capital_holding_slot(this->get_settlement_holding_slots().front());
	}

	//ensure the province's settlement holding slots have been initialized, so that its culture and religion will be calculated correctly
	for (holding_slot *settlement_holding_slot : this->get_settlement_holding_slots()) {
		if (!settlement_holding_slot->is_history_initialized()) {
			settlement_holding_slot->initialize_history();
		}
	}

	if (this->get_county() != nullptr) {
		this->calculate_population();
		this->calculate_population_groups();

		if (this->has_river()) {
			this->change_population_capacity_additive_modifier(10000); //increase population capacity if this province has a river
		}
		if (this->is_coastal()) {
			this->change_population_capacity_additive_modifier(10000); //increase population capacity if this province is coastal
		}

		this->calculate_trade_node();

		if (!this->can_have_trading_post()) {
			//destroy the created trading post holding slot if the province can't currently have a trading post
			this->destroy_trading_post_holding_slot();
		}
	}

	data_entry_base::initialize_history();
}

void province::check() const
{
	if (this->get_terrain() == nullptr) {
		throw std::runtime_error("Province \"" + this->get_identifier() + "\" has no terrain.");
	}

	if (!this->get_color().isValid()) {
		throw std::runtime_error("Province \"" + this->get_identifier() + "\" has no valid color.");
	}

	if (this->get_county() != nullptr) {
		if (this->get_settlement_holding_slots().empty()) {
			throw std::runtime_error("Province \"" + this->get_identifier() + "\" has a county (not being a wasteland or water zone), but has no settlement holding slots.");
		}
	}

	if (static_cast<int>(this->get_settlement_holding_slots().size()) > defines::get()->get_max_settlement_slots_per_province()) {
		throw std::runtime_error("Province \"" + this->get_identifier() + "\" has " + std::to_string(this->get_settlement_holding_slots().size()) + " settlement slots, but the maximum settlement slots per province is set to " + std::to_string(defines::get()->get_max_settlement_slots_per_province()) + ".");
	}

	if (static_cast<int>(this->get_palace_holding_slots().size()) > defines::get()->get_max_palace_slots_per_province()) {
		throw std::runtime_error("Province \"" + this->get_identifier() + "\" has " + std::to_string(this->get_palace_holding_slots().size()) + " palace slots, but the maximum palace slots per province is set to " + std::to_string(defines::get()->get_max_palace_slots_per_province()) + ".");
	}
}

void province::check_history() const
{
	/*
	if (this->border_provinces.empty()) {
		throw std::runtime_error("Province \"" + this->get_identifier() + "\" has no border provinces.");
	}
	*/

	if (this->get_county() != nullptr && !this->get_settlement_holdings().empty()) {
		if (this->get_culture() == nullptr) {
			throw std::runtime_error("Province \"" + this->get_identifier() + "\" has no culture.");
		}

		if (this->get_religion() == nullptr) {
			throw std::runtime_error("Province \"" + this->get_identifier() + "\" has no religion.");
		}
	}

	if (this->get_trade_node() == nullptr && this->get_owner() != nullptr) {
		throw std::runtime_error("Province \"" + this->get_identifier() + "\" has an owner, but is not assigned to any trade node.");
	}

	if (this->get_capital_holding_slot() != nullptr && this->get_capital_holding_slot()->get_province() != this) {
		throw std::runtime_error("Province \"" + this->get_identifier() + "\"'s capital holding slot (\"" + this->get_capital_holding_slot()->get_barony()->get_identifier() + "\") belongs to another province (\"" + this->get_capital_holding_slot()->get_province()->get_identifier() + "\").");
	}

	try {
		for (const qunique_ptr<wildlife_unit> &wildlife_unit : this->get_wildlife_units()) {
			wildlife_unit->check_history();
		}
	} catch (...) {
		std::throw_with_nested(std::runtime_error("A wildlife unit in province \"" + this->get_identifier() + "\" is in an invalid state."));
	}

	this->check();
}

gsml_data province::get_cache_data() const
{
	gsml_data cache_data(this->get_identifier());
	if (this->get_terrain() != nullptr) {
		cache_data.add_property("terrain", this->get_terrain()->get_identifier());
	}

	gsml_data border_provinces("border_provinces");
	for (const province *province : this->border_provinces) {
		border_provinces.add_value(province->get_identifier());
	}
	cache_data.add_child(std::move(border_provinces));

	for (const holding_slot *slot : this->get_settlement_holding_slots()) {
		gsml_data slot_cache_data = slot->get_cache_data();

		if (!slot_cache_data.is_empty()) {
			cache_data.add_child(std::move(slot_cache_data));
		}
	}

	if (!this->get_path_pos_list().empty()) {
		gsml_data path_pos_list_data("path_pos_list");
		for (const QPoint &path_pos : this->get_path_pos_list()) {
			path_pos_list_data.add_child(gsml_data::from_point(path_pos));
		}
		cache_data.add_child(std::move(path_pos_list_data));
	}

	return cache_data;
}

void province::do_day()
{
}

void province::do_month()
{
	this->calculate_population_groups();

	if (this->trade_node_recalculation_needed) {
		this->calculate_trade_node();
		this->set_trade_node_recalculation_needed(false);
	}
}

std::string province::get_name() const
{
	if (this->get_county() != nullptr) {
		return translator::get()->translate(this->get_county()->get_identifier_with_aliases(), this->get_tag_suffix_list_with_fallbacks());
	}

	return translator::get()->translate(this->get_identifier_with_aliases()); //province without a county; sea zone, river, lake or wasteland
}

std::vector<std::vector<std::string>> province::get_tag_suffix_list_with_fallbacks() const
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

void province::set_county(landed_title *county)
{
	if (county == this->get_county()) {
		return;
	}

	this->county = county;
	county->set_province(this);
	emit county_changed();
}

/**
**	@brief	Get the province's de facto duchy
**
**	@return	The province's de facto duchy
*/
landed_title *province::get_duchy() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->get_duchy();
	}

	return nullptr;
}

landed_title *province::get_de_jure_duchy() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->get_de_jure_duchy();
	}

	return nullptr;
}

/**
**	@brief	Get the province's de facto kingdom
**
**	@return	The province's de facto kingdom
*/
landed_title *province::get_kingdom() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->get_kingdom();
	}

	return nullptr;
}

landed_title *province::get_de_jure_kingdom() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->get_de_jure_kingdom();
	}

	return nullptr;
}

/**
**	@brief	Get the province's (de facto) empire
**
**	@return	The province's (de facto) empire
*/
landed_title *province::get_empire() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->get_empire();
	}

	return nullptr;
}

landed_title *province::get_de_jure_empire() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->get_de_jure_empire();
	}

	return nullptr;
}

void province::set_world(metternich::world *world)
{
	if (world == this->get_world()) {
		return;
	}

	this->world = world;
}

void province::set_trade_node(metternich::trade_node *trade_node)
{
	if (trade_node == this->get_trade_node()) {
		return;
	}

	if (this->get_trade_node() != nullptr) {
		this->get_trade_node()->remove_province(this);
	}

	this->trade_node = trade_node;
	emit trade_node_changed();

	if (trade_node != nullptr) {
		trade_node->add_province(this);
	}

	if (trade_node == nullptr || this->is_center_of_trade()) {
		this->set_trade_node_trade_cost(0);
	}

	if (map::get()->get_mode() == map_mode::trade_node) {
		this->update_color_for_map_mode(map::get()->get_mode());
	}
}

void province::calculate_trade_node()
{
	if (this->is_center_of_trade()) {
		//the trade node of centers of trade cannot change, since trade nodes represent a collective of provinces which have a province as their center of trade
		return;
	}

	if (this->get_owner() == nullptr) {
		//provinces without an owner don't get assigned to any trade node
		this->set_trade_node(nullptr);
		return;
	}

	std::pair<metternich::trade_node *, int> best_trade_node_result = this->get_best_trade_node_from_list(this->get_world()->get_active_trade_nodes());
	this->set_trade_node(best_trade_node_result.first);
	this->set_trade_node_trade_cost(best_trade_node_result.second);
}

std::pair<trade_node *, int> province::get_best_trade_node_from_list(const std::set<metternich::trade_node *> &trade_nodes) const
{
	std::vector<metternich::trade_node *> sorted_trade_nodes = container::to_vector(trade_nodes);

	std::sort(sorted_trade_nodes.begin(), sorted_trade_nodes.end(), [this](const metternich::trade_node *a, const metternich::trade_node *b) {
		return this->get_kilometers_distance_to(a->get_center_of_trade()) < this->get_kilometers_distance_to(b->get_center_of_trade());
	});

	metternich::trade_node *best_node = nullptr;
	int best_score = 0; //smaller is better
	int best_trade_cost = 0;

	for (metternich::trade_node *node : sorted_trade_nodes) {
		if (node->get_world() != this->get_world()) {
			continue;
		}

		province *center_of_trade = node->get_center_of_trade();

		//the minimum, best-case trade cost that will be incurred between this province and the center of trade
		const int distance = this->get_kilometers_distance_to(center_of_trade);
		const int minimum_trade_cost = distance * 100 / province::base_distance * defines::get()->get_trade_cost_modifier_per_distance() / 100;

		if (best_node != nullptr && minimum_trade_cost >= best_score) {
			continue;
		}

		const pathfinder *pathfinder = this->get_world()->get_pathfinder();
		const find_trade_path_result result = pathfinder->find_trade_path(this, center_of_trade);
		if (!result.success) {
			continue;
		}

		int score = result.trade_cost; //smaller is better

		int score_modifier = 100;

		if (this->get_county()->get_realm() != center_of_trade->get_county()->get_realm()) {
			score_modifier += defines::get()->get_trade_node_score_realm_modifier();
		}

		if (this->get_culture() != center_of_trade->get_culture()) {
			score_modifier += defines::get()->get_trade_node_score_culture_modifier();
		}

		if (this->get_culture()->get_culture_group() != center_of_trade->get_culture()->get_culture_group()) {
			score_modifier += defines::get()->get_trade_node_score_culture_group_modifier();
		}

		if (this->get_religion() != center_of_trade->get_religion()) {
			score_modifier += defines::get()->get_trade_node_score_religion_modifier();
		}

		if (this->get_religion()->get_religion_group() != center_of_trade->get_religion()->get_religion_group()) {
			score_modifier += defines::get()->get_trade_node_score_religion_group_modifier();
		}

		score_modifier = std::max(0, score_modifier);

		score *= score_modifier;
		score /= 100;

		if (best_node == nullptr || score < best_score) {
			best_node = node;
			best_score = score;
			best_trade_cost = result.trade_cost;
		}
	}

	return std::make_pair(best_node, best_trade_cost);
}

const QColor &province::get_color_for_map_mode(const map_mode mode) const
{
	if (this->get_county() != nullptr) {
		switch (mode) {
			case map_mode::country: {
				const landed_title *realm = this->get_county()->get_realm();
				if (realm != nullptr) {
					return realm->get_color();
				}
				break;
			}
			case map_mode::de_jure_empire: {
				const landed_title *empire = this->get_de_jure_empire();
				if (empire != nullptr) {
					return empire->get_color();
				}
				break;
			}
			case map_mode::de_jure_kingdom: {
				const landed_title *kingdom = this->get_de_jure_kingdom();
				if (kingdom != nullptr) {
					return kingdom->get_color();
				}
				break;
			}
			case map_mode::de_jure_duchy: {
				const landed_title *duchy = this->get_de_jure_duchy();
				if (duchy != nullptr) {
					return duchy->get_color();
				}
				break;
			}
			case map_mode::culture: {
				if (this->get_culture() != nullptr) {
					return this->get_culture()->get_color();
				}
				break;
			}
			case map_mode::culture_group: {
				if (this->get_culture() != nullptr) {
					return this->get_culture()->get_culture_group()->get_color();
				}
				break;
			}
			case map_mode::religion: {
				if (this->get_religion() != nullptr) {
					return this->get_religion()->get_color();
				}
				break;
			}
			case map_mode::religion_group: {
				if (this->get_religion() != nullptr) {
					return this->get_religion()->get_religion_group()->get_color();
				}
				break;
			}
			case map_mode::trade_node: {
				if (this->get_trade_node() != nullptr && this->get_owner() != nullptr) {
					return this->get_trade_node()->get_color();
				}
				break;
			}
			default:
				break;
		}

		switch (mode) {
			case map_mode::country:
			case map_mode::culture:
			case map_mode::culture_group:
			case map_mode::religion:
			case map_mode::religion_group:
			case map_mode::trade_node:
				return province::empty_province_color; //colonizable province
			default:
				break;
		}
	}

	if (this->is_water()) {
		return province::water_province_color;
	} else {
		return province::wasteland_province_color;
	}
}

void province::create_image(const std::vector<int> &pixel_indexes)
{
	this->pixel_count = static_cast<int>(pixel_indexes.size());

	QPoint start_pos(-1, -1);
	QPoint end_pos(-1, -1);
	long long int center_x = 0;
	long long int center_y = 0;

	for (const int index : pixel_indexes) {
		QPoint pixel_pos = this->get_world()->get_pixel_pos(index);

		if (start_pos.x() == -1 || pixel_pos.x() < start_pos.x()) {
			start_pos.setX(pixel_pos.x());
		}
		if (start_pos.y() == -1 || pixel_pos.y() < start_pos.y()) {
			start_pos.setY(pixel_pos.y());
		}
		if (end_pos.x() == -1 || pixel_pos.x() > end_pos.x()) {
			end_pos.setX(pixel_pos.x());
		}
		if (end_pos.y() == -1 || pixel_pos.y() > end_pos.y()) {
			end_pos.setY(pixel_pos.y());
		}

		center_x += pixel_pos.x();
		center_y += pixel_pos.y();
	}

	center_x /= this->pixel_count;
	center_y /= this->pixel_count;
	QPoint center_pos(static_cast<int>(center_x), static_cast<int>(center_y));

	this->rect = QRect(start_pos, end_pos);

	this->image = QImage(this->rect.size(), QImage::Format_Indexed8);

	//index 0 = transparency, index 1 = the main color for the province, index 2 = province border
	this->image.setColorTable({qRgba(0, 0, 0, 0), this->get_color().rgb(), QColor(Qt::darkGray).rgb()});
	this->image.fill(0);

	for (const int index : pixel_indexes) {
		QPoint pixel_pos = this->get_world()->get_pixel_pos(index) - this->rect.topLeft();
		this->image.setPixel(pixel_pos, 1);
	}

	//if the center pos is outside the actual pixels of the province, change it to the nearest pixel actually in the province
	this->center_pos = this->get_nearest_valid_pos(center_pos);
}

void province::set_border_pixels(const std::vector<int> &pixel_indexes)
{
	for (const int index : pixel_indexes) {
		QPoint pixel_pos = this->get_world()->get_pixel_pos(index) - this->rect.topLeft();
		this->image.setPixel(pixel_pos, 2);
	}
}

void province::update_image()
{
	const QColor &province_color = this->get_map_mode_color();

	QColor border_color;
	if (this->is_selected()) {
		//if the province is selected, highlight its border pixels
		border_color = QColor(Qt::yellow);
	} else {
		border_color = QColor(Qt::black);
	}

	this->image.setColor(1, province_color.rgb());
	this->image.setColor(2, border_color.rgb());

	emit image_changed();
}

void province::write_geodata_to_image(QImage &image, QImage &terrain_image) const
{
	for (const QGeoPolygon &geopolygon : this->geopolygons) {
		this->write_geoshape_to_image(image, geopolygon, terrain_image);
	}

	for (const QGeoPath &geopath : this->geopaths) {
		this->write_geoshape_to_image(image, geopath, terrain_image);
	}
}

void province::write_geopath_endpoints_to_image(QImage &image, QImage &terrain_image) const
{
	const int circle_radius = this->get_terrain()->get_path_width() / 2;

	for (const QGeoPath &geopath : this->geopaths) {
		QGeoCircle front_geocircle(geopath.path().front(), circle_radius);
		this->write_geoshape_to_image(image, front_geocircle, terrain_image);

		QGeoCircle back_geocircle(geopath.path().back(), circle_radius);
		this->write_geoshape_to_image(image, back_geocircle, terrain_image);
	}
}

void province::write_geoshape_to_image(QImage &image, const QGeoShape &geoshape, QImage &terrain_image) const
{
	const QString province_loading_message = engine_interface::get()->get_loading_message();

	QRgb rgb = this->get_color().rgb();
	QRgb *rgb_data = reinterpret_cast<QRgb *>(image.bits());

	QRgb terrain_rgb = qRgb(0, 0, 0);
	if (this->get_terrain() != nullptr) {
		terrain_rgb = this->get_terrain()->get_color().rgb();
	} else if (terrain_type::get_default_terrain() != nullptr) {
		terrain_rgb = terrain_type::get_default_terrain()->get_color().rgb();
	}

	QRgb *terrain_rgb_data = reinterpret_cast<QRgb *>(terrain_image.bits());

	const double lon_per_pixel = 360.0 / static_cast<double>(image.size().width());
	const double lat_per_pixel = 180.0 / static_cast<double>(image.size().height());

	QGeoRectangle georectangle = geoshape.boundingGeoRectangle();
	QGeoCoordinate bottom_left = georectangle.bottomLeft();
	QGeoCoordinate top_right = georectangle.topRight();

	if (geoshape.type() == QGeoShape::ShapeType::PathType) {
		//increase the bounding rectangle of paths slightly, as otherwise a part of the path's width is cut off
		bottom_left.setLatitude(bottom_left.latitude() - 0.1);
		bottom_left.setLongitude(bottom_left.longitude() - 0.1);
		top_right.setLatitude(top_right.latitude() + 0.1);
		top_right.setLongitude(top_right.longitude() + 0.1);
	}

	const double start_lon = bottom_left.longitude();
	const double end_lon = top_right.longitude();

	double lon = start_lon;
	lon = geocoordinate::longitude_to_pixel_longitude(lon, lon_per_pixel);
	const int start_x = geocoordinate::longitude_to_x(lon, lon_per_pixel);

	const double start_lat = bottom_left.latitude();
	const double end_lat = top_right.latitude();
	const double normalized_start_lat = geocoordinate::latitude_to_pixel_latitude(start_lat, lat_per_pixel);

	const int pixel_width = static_cast<int>(std::round((std::abs(end_lon - start_lon)) / lon_per_pixel));
	const bool show_progress = pixel_width >= 512;

	for (; lon <= end_lon; lon += lon_per_pixel) {
		const int x = geocoordinate::longitude_to_x(lon, lon_per_pixel);

		for (double lat = normalized_start_lat; lat <= end_lat; lat += lat_per_pixel) {
			QGeoCoordinate coordinate(lat, lon);

			const int y = geocoordinate::latitude_to_y(lat, lat_per_pixel);
			const int pixel_index = point::to_index(x, y, image.size());

			//only write the province to the pixel if it is empty, or if this is a river province and the province to overwrite is not an ocean province
			if (rgb_data[pixel_index] != province::empty_rgb && (!this->is_river() || province::get_by_rgb(rgb_data[pixel_index])->is_ocean())) {
				continue;
			}

			if (!geoshape.contains(coordinate)) {
				continue;
			}

			rgb_data[pixel_index] = rgb;

			if (terrain_rgb_data[pixel_index] == terrain_type::empty_rgb || this->is_river()) {
				terrain_rgb_data[pixel_index] = terrain_rgb;
			}
		}

		if (show_progress) {
			const int progress_percent = (x - start_x) * 100 / pixel_width;
			engine_interface::get()->set_loading_message(province_loading_message + "\nWriting Geoshape for the " + QString::fromStdString(this->get_name()) + " Province to Image... (" + QString::number(progress_percent) + "%)");
		}
	}

	engine_interface::get()->set_loading_message(province_loading_message);
}

void province::write_geojson() const
{
	QVariantList top_list;

	QVariantMap feature_collection;
	feature_collection["type"] = "FeatureCollection";

	QVariantList features;

	QVariantMap feature;
	QVariantMap feature_properties;
	feature_properties["name"] = QString::fromStdString(this->get_identifier());
	feature["properties"] = feature_properties;

	QVariantList shapes;

	if (!this->geopolygons.empty()) {
		feature["type"] = "MultiPolygon";

		for (const QGeoPolygon &geopolygon : this->geopolygons) {
			QVariantMap polygon;
			polygon["type"] = "Polygon";
			polygon["data"] = QVariant::fromValue(geopolygon);
			shapes.push_back(polygon);
		}
	} else {
		feature["type"] = "MultiLineString";

		for (const QGeoPath &geopath : this->geopaths) {
			QVariantMap line;
			line["type"] = "LineString";
			line["data"] = QVariant::fromValue(geopath);
			shapes.push_back(line);
		}
	}

	feature["data"] = shapes;

	features.push_back(feature);
	feature_collection["data"] = features;

	top_list.push_back(feature_collection);

	QJsonDocument geojson = QGeoJson::exportGeoJson(top_list);
	std::filesystem::path filepath = database::get_map_path() / (this->get_identifier() + ".geojson");
	std::ofstream ofstream(filepath);
	ofstream << geojson.toJson().constData();
}

void province::set_terrain(metternich::terrain_type *terrain)
{
	if (terrain == this->get_terrain()) {
		return;
	}

	const metternich::terrain_type *old_terrain = this->get_terrain();

	if (old_terrain != nullptr) {
		if (old_terrain->get_modifier() != nullptr) {
			old_terrain->get_modifier()->remove(this);
		}
	}

	this->terrain = terrain;

	if (terrain != nullptr) {
		if (terrain->get_modifier() != nullptr) {
			terrain->get_modifier()->apply(this);
		}
	}

	emit terrain_changed();
}

character *province::get_owner() const
{
	if (this->get_county() != nullptr) {
		return this->get_county()->get_holder();
	}

	return nullptr;
}

void province::set_culture(metternich::culture *culture)
{
	if (culture == this->get_culture()) {
		return;
	}

	if (culture == nullptr && this->get_owner() != nullptr) {
		throw std::runtime_error("Tried to set the culture of province \"" + this->get_identifier() + "\" to null, despite it having an owner.");
	}

	const metternich::culture *old_culture = this->get_culture();
	metternich::culture_group *old_culture_group = old_culture ? old_culture->get_culture_group() : nullptr;

	this->culture = culture;
	emit culture_changed();
	metternich::culture_group *culture_group = culture ? culture->get_culture_group() : nullptr;

	this->set_trade_node_recalculation_needed(true);

	if (
		map::get()->get_mode() == map_mode::culture
		|| (map::get()->get_mode() == map_mode::culture_group && old_culture_group != culture_group)
	) {
		this->update_color_for_map_mode(map::get()->get_mode());
	}
}

void province::set_religion(metternich::religion *religion)
{
	if (religion == this->get_religion()) {
		return;
	}

	if (religion == nullptr && this->get_owner() != nullptr) {
		throw std::runtime_error("Tried to set the religion of province \"" + this->get_identifier() + "\" to null, despite it having an owner.");
	}

	metternich::religion *old_religion = this->get_religion();
	metternich::religion_group *old_religion_group = old_religion ? old_religion->get_religion_group() : nullptr;

	this->religion = religion;
	emit religion_changed();
	metternich::religion_group *religion_group = religion ? religion->get_religion_group() : nullptr;

	this->set_trade_node_recalculation_needed(true);

	if (
		map::get()->get_mode() == map_mode::religion
		|| (map::get()->get_mode() == map_mode::religion_group && old_religion_group != religion_group)
	) {
		this->update_color_for_map_mode(map::get()->get_mode());
	}
}

void province::set_population(const int population)
{
	if (population == this->get_population()) {
		return;
	}

	this->population = population;
	emit population_changed();
}

void province::calculate_population()
{
	int population = 0;
	for (const holding *holding : this->get_settlement_holdings()) {
		population += holding->get_population();
	}
	this->set_population(population);
}

void province::set_population_capacity_additive_modifier(const int population_capacity_modifier)
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

void province::set_population_capacity_modifier(const int population_capacity_modifier)
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

void province::set_population_growth_modifier(const int population_growth_modifier)
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

void province::calculate_population_groups()
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

	//update the province's main culture and religion

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

holding_slot *province::get_holding_slot(const std::string &holding_slot_str) const
{
	if (holding_slot_str.substr(0, 2) == holding_slot::prefix) {
		holding_slot *holding_slot = holding_slot::get(holding_slot_str);

		if (holding_slot->get_province() != this) {
			throw std::runtime_error("Tried to get holding slot \"" + holding_slot->get_identifier() + "\" for province \"" + this->get_identifier() + "\", but the holding slot belongs to another province.");
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
					throw std::runtime_error("Tried to get the trading post holding slot of province \"" + this->get_identifier() + "\", which has no such slot.");
				}

				return this->get_trading_post_holding_slot();
			case holding_slot_type::factory:
				return this->get_factory_holding_slot();
			default:
				break;
		}
	}

	throw std::runtime_error("\"" + holding_slot_str + "\" is not a valid holding slot string for province history.");
}

void province::add_holding_slot(holding_slot *holding_slot)
{
	switch (holding_slot->get_type()) {
		case holding_slot_type::settlement:
			this->settlement_holding_slots.push_back(holding_slot);
			//add the holding slot to its province's regions
			for (region *region : this->get_regions()) {
				region->add_holding(holding_slot);
			}
			emit settlement_holding_slots_changed();
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
			throw std::runtime_error("Holding slot \"" + holding_slot->get_identifier() + "\" has an invalid type (" + std::to_string(static_cast<int>(holding_slot->get_type())) + "), but is being added to province \"" + this->get_identifier() + "\".");
	}
}

QVariantList province::get_settlement_holding_slots_qvariant_list() const
{
	return container::to_qvariant_list(this->get_settlement_holding_slots());
}

QVariantList province::get_settlement_holdings_qvariant_list() const
{
	return container::to_qvariant_list(this->get_settlement_holdings());
}

void province::create_holding(holding_slot *holding_slot, holding_type *type)
{
	auto new_holding = std::make_unique<holding>(holding_slot, type);
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

void province::destroy_holding(holding_slot *holding_slot)
{
	holding *holding = holding_slot->get_holding();

	if (holding_slot->get_type() == holding_slot_type::settlement) {
		if (holding == this->get_capital_holding()) {
			//if the capital holding is being destroyed, set the next holding as the capital, if any exists, or otherwise set the capital holding to null
			if (this->settlement_holdings.size() > 1) {
				this->set_capital_holding(this->settlement_holdings.at(1));
			} else {
				this->set_capital_holding(nullptr);
			}
		}

		this->settlement_holdings.erase(std::remove(this->settlement_holdings.begin(), this->settlement_holdings.end(), holding), this->settlement_holdings.end());
		emit settlement_holdings_changed();
	}

	holding_slot->set_holding(nullptr);
}

void province::set_capital_holding_slot(holding_slot *holding_slot)
{
	if (holding_slot == this->get_capital_holding_slot()) {
		return;
	}

	const QPoint &old_main_pos = this->get_main_pos();

	if (holding_slot != nullptr) {
		if (holding_slot->get_province() != this) {
			throw std::runtime_error("Tried to set holding \"" + holding_slot->get_identifier() + "\" as the capital holding of province \"" + this->get_identifier() + "\", but it belongs to another province.");
		}

		if (holding_slot->get_type() != holding_slot_type::settlement) {
			throw std::runtime_error("Tried to set holding \"" + holding_slot->get_identifier() + "\" as the capital holding of province \"" + this->get_identifier() + "\", but it is not a settlement holding.");
		}
	}

	this->capital_holding_slot = holding_slot;
	emit capital_holding_slot_changed();

	const QPoint &main_pos = this->get_main_pos();

	if (old_main_pos != main_pos) {
		emit main_pos_changed();
	}
}

holding *province::get_capital_holding() const
{
	if (this->get_capital_holding_slot() != nullptr) {
		return this->get_capital_holding_slot()->get_holding();
	}

	return nullptr;
}

void province::set_capital_holding(holding *holding)
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

QVariantList province::get_palace_holding_slots_qvariant_list() const
{
	return container::to_qvariant_list(this->get_palace_holding_slots());
}

std::string province::get_trading_post_holding_slot_identifier() const
{
	return holding_slot::prefix + this->get_identifier_without_prefix() + "_trading_post";
}

void province::create_trading_post_holding_slot()
{
	std::string holding_slot_identifier = this->get_trading_post_holding_slot_identifier();
	holding_slot *holding_slot = holding_slot::add(holding_slot_identifier);
	holding_slot->set_type(holding_slot_type::trading_post);
	holding_slot->set_province(this);
	emit trading_post_holding_slot_changed();
}

void province::destroy_trading_post_holding_slot()
{
	std::string holding_slot_identifier = this->get_trading_post_holding_slot_identifier();
	holding_slot::remove(holding_slot_identifier);
	this->trading_post_holding_slot = nullptr;
	emit trading_post_holding_slot_changed();
}

void province::add_population_unit(qunique_ptr<population_unit> &&population_unit)
{
	this->population_units.push_back(std::move(population_unit));
}

void province::add_wildlife_unit(qunique_ptr<wildlife_unit> &&wildlife_unit)
{
	this->wildlife_units.push_back(std::move(wildlife_unit));
	emit wildlife_units_changed();
}

QVariantList province::get_wildlife_units_qvariant_list() const
{
	QVariantList list;

	for (const qunique_ptr<wildlife_unit> &wildlife_unit : this->get_wildlife_units()) {
		list.append(QVariant::fromValue(wildlife_unit.get()));
	}

	return list;
}

void province::sort_wildlife_units()
{
	std::sort(this->wildlife_units.begin(), this->wildlife_units.end(), [](const qunique_ptr<wildlife_unit> &a, const qunique_ptr<wildlife_unit> &b) {
		//give priority to population units with greater size, so that they will be displayed first
		return a->get_size() > b->get_size();
	});

	emit wildlife_units_changed();
}

void province::remove_empty_wildlife_units()
{
	bool removed_pop_unit = false;
	for (size_t i = 0; i < this->wildlife_units.size();) {
		const qunique_ptr<wildlife_unit> &wildlife_unit = this->wildlife_units[i];
		if (wildlife_unit->get_size() == 0) {
			this->wildlife_units.erase(this->wildlife_units.begin() + static_cast<int>(i));
			removed_pop_unit = true;
		} else {
			++i;
		}
	}

	if (removed_pop_unit) {
		emit wildlife_units_changed();
	}
}

bool province::borders_water() const
{
	for (const province *border_province : this->border_provinces) {
		if (border_province->is_water()) {
			return true;
		}
	}

	return false;
}

bool province::borders_river() const
{
	for (const province *border_province : this->border_provinces) {
		if (border_province->is_river()) {
			return true;
		}
	}

	return false;
}

bool province::has_river() const
{
	return this->has_inner_river() || this->borders_river();
}

bool province::is_coastal() const
{
	for (const province *border_province : this->border_provinces) {
		if (border_province->is_ocean()) {
			return true;
		}
	}

	return false;
}

bool province::is_water() const
{
	return this->get_terrain() != nullptr && this->get_terrain()->is_water();
}

bool province::is_ocean() const
{
	return this->get_terrain() != nullptr && this->get_terrain()->is_ocean();
}

bool province::is_river() const
{
	return this->get_terrain() != nullptr && this->get_terrain()->is_river();
}

QVariantList province::get_technologies_qvariant_list() const
{
	return container::to_qvariant_list(this->get_technologies());
}

void province::add_technology(technology *technology)
{
	if (history::get()->is_loading()) {
		//if is loading history, automatically add all prerequisites when adding a technology
		for (metternich::technology *required_technology : technology->get_required_technologies()) {
			if (!this->has_technology(required_technology)) {
				this->add_technology(required_technology);
			}
		}
	}

	this->technologies.insert(technology);
	emit technologies_changed();
}

void province::remove_technology(technology *technology)
{
	this->technologies.erase(technology);
	emit technologies_changed();
}

bool province::is_center_of_trade() const
{
	return this->get_trade_node() != nullptr && this->get_trade_node()->get_center_of_trade() == this;
}

void province::add_active_trade_route(trade_route *route)
{
	this->active_trade_routes.insert(route);
	emit active_trade_routes_changed();

	if (this->can_have_trading_post() && this->get_trading_post_holding_slot() == nullptr) {
		this->create_trading_post_holding_slot();
	}
}

void province::remove_active_trade_route(trade_route *route)
{
	this->active_trade_routes.erase(route);
	emit active_trade_routes_changed();

	if (!this->can_have_trading_post() && this->get_trading_post_holding_slot() != nullptr) {
		this->destroy_trading_post_holding_slot();
	}
}

bool province::has_trade_route_connection_to(const province *other_province) const
{
	for (trade_route *route : this->active_trade_routes) {
		if (!other_province->has_trade_route(route)) {
			continue;
		}

		if (route->has_connection_between(this, other_province)) {
			return true;
		}
	}

	return false;
}

bool province::has_any_trade_route_land_connection() const
{
	for (trade_route *route : this->active_trade_routes) {
		if (route->has_any_land_connection_for_province(this)) {
			return true;
		}
	}

	return false;
}

void province::set_selected(const bool selected, const bool notify_engine_interface)
{
	if (selected == this->is_selected()) {
		return;
	}

	if (selected) {
		if (province::selected_province != nullptr) {
			province::selected_province->set_selected(false, false);
		}
		province::selected_province = this;
	} else {
		province::selected_province = nullptr;
	}

	this->selected = selected;
	emit selected_changed();

	this->update_image();

	if (notify_engine_interface) {
		engine_interface::get()->emit selected_province_changed();
	}
}

bool province::is_selectable() const
{
	return this->get_county() != nullptr;
}

QVariantList province::get_population_per_type_qvariant_list() const
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

QVariantList province::get_population_per_culture_qvariant_list() const
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

QVariantList province::get_population_per_religion_qvariant_list() const
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

QVariantList province::get_geopolygons_qvariant_list() const
{
	return container::to_qvariant_list(this->get_geopolygons());
}

QVariantList province::get_geopaths_qvariant_list() const
{
	return container::to_qvariant_list(this->geopaths);
}

QGeoCoordinate province::get_center_coordinate() const
{
	return this->get_world()->get_pixel_pos_coordinate(this->get_center_pos());
}

const QPoint &province::get_main_pos() const
{
	if (this->get_capital_holding_slot() != nullptr && this->get_capital_holding_slot()->get_pos().x() != -1 && this->get_capital_holding_slot()->get_pos().y() != -1) {
		return this->get_capital_holding_slot()->get_pos();
	}

	return this->get_center_pos();
}

QPoint province::get_nearest_valid_pos(const QPoint &pos) const
{
	if (this->image.isNull()) {
		throw std::runtime_error("Tried to get the nearest valid position for point (" + std::to_string(pos.x()) + ", " + std::to_string(pos.y()) + ") in province \"" + this->get_identifier() + "\", but the latter has no valid image to check the position of pixels with.");
	}

	QPoint start_pos = this->rect.topLeft();
	if (this->is_valid_pos(pos)) {
		return pos; //the pos itself is already a valid position, so return the pos itself
	}

	//get the nearest position to a point that is actually a position inside the province

	int offset = 0;
	bool checked_pos_in_rect = true; //whether a position within the province's rectangle was checked in the loop
	bool checked_any_pos_in_rect = false; //whether any position in the province's rectangle has been checked so far

	while (checked_pos_in_rect || !checked_any_pos_in_rect) {
		offset++;
		checked_pos_in_rect = false;

		for (int x_offset = -offset; x_offset <= offset; ++x_offset) {
			int y_offset = -(offset - std::abs(x_offset));

			for (int y_offset_multiplier = -1; y_offset_multiplier <= 1; y_offset_multiplier += 2) {
				y_offset *= y_offset_multiplier;

				QPoint near_pos = pos + QPoint(x_offset, y_offset);
				if (this->rect.contains(near_pos)) {
					checked_pos_in_rect = true;
					checked_any_pos_in_rect = true;

					if (this->image.pixel(near_pos - start_pos) != qRgba(0, 0, 0, 0)) {
						return near_pos;
					}
				}

				// do the same with the inverted coordinate position of the offsets
				near_pos = pos + QPoint(y_offset, x_offset);
				if (this->rect.contains(near_pos)) {
					checked_pos_in_rect = true;
					checked_any_pos_in_rect = true;

					if (this->image.pixel(near_pos - start_pos) != qRgba(0, 0, 0, 0)) {
						return near_pos;
					}
				}

				if (y_offset == 0) {
					break; //no need to do it a second time if the y offset is 0
				}
			}
		}
	}

	throw std::runtime_error("Could not find a nearest valid position for point (" + std::to_string(pos.x()) + ", " + std::to_string(pos.y()) + ") in province \"" + this->get_identifier() + "\".");
}

void province::set_trade_node_recalculation_needed(const bool recalculation_needed, const bool recalculate_for_dependent_provinces)
{
	if (recalculate_for_dependent_provinces && this->is_center_of_trade() && recalculation_needed) {
		for (province *node_province : this->get_trade_node()->get_provinces()) {
			if (node_province == this) {
				continue;
			}

			node_province->set_trade_node_recalculation_needed(true);
		}
	}

	if (recalculation_needed == this->trade_node_recalculation_needed) {
		return;
	}

	this->trade_node_recalculation_needed = recalculation_needed;
}

}
