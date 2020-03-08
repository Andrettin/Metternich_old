#include "map/province.h"

#include "character/character.h"
#include "culture/culture.h"
#include "culture/culture_group.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "defines.h"
#include "economy/trade_node.h"
#include "economy/trade_route.h"
#include "engine_interface.h"
#include "game/game.h"
#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "holding/holding_slot_type.h"
#include "landed_title/landed_title.h"
#include "map/map.h"
#include "map/map_mode.h"
#include "map/pathfinder.h"
#include "map/terrain_type.h"
#include "map/world.h"
#include "religion/religion.h"
#include "religion/religion_group.h"
#include "script/modifier.h"
#include "species/wildlife_unit.h"
#include "util/container_util.h"
#include "util/geocoordinate_util.h"
#include "util/point_util.h"
#include "util/rect_util.h"

#include <QApplication>
#include <QGeoCircle>
#include <QtLocation/private/qgeojson_p.h>

namespace metternich {

std::set<std::string> province::get_database_dependencies()
{
	return {
		//so that baronies will be ensured to exist when provinces (and thus holding slots) are processed
		landed_title::class_identifier,
		//so that megalopolis holding slots will be ensured to exist when provinces are processed
		world::class_identifier,
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

province::province(const std::string &identifier) : territory(identifier)
{
	connect(this, &province::culture_changed, this, &identifiable_data_entry_base::name_changed);
	connect(this, &province::religion_changed, this, &identifiable_data_entry_base::name_changed);
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
		scope.for_each_child([&](const gsml_data &polygon_data) {
			this->geopolygons.push_back(polygon_data.to_geopolygon());
		});
	} else if (tag == "geopaths") {
		scope.for_each_child([&](const gsml_data &path_data) {
			QGeoPath geopath = path_data.to_geopath();
			if (this->get_terrain() != nullptr) {
				geopath.setWidth(this->get_terrain()->get_path_width());
			}
			this->geopaths.push_back(std::move(geopath));
		});
	} else if (tag == "border_provinces") {
		for (const std::string &border_province_identifier : scope.get_values()) {
			province *border_province = province::get(border_province_identifier);
			this->border_provinces.insert(border_province);
		}
	} else if (tag == "river_crossings") {
		for (const std::string &border_province_identifier : scope.get_values()) {
			province *border_province = province::get(border_province_identifier);
			this->river_crossings.insert(border_province);
		}
	} else if (tag == "path_pos_map") {
		scope.for_each_child([&](const gsml_data &pos_list_data) {
			const province *other_province = province::get(pos_list_data.get_tag());
			std::vector<QPointF> pos_list;
			pos_list_data.for_each_child([&](const gsml_data &pos_data) {
				pos_list.push_back(pos_data.to_point());
			});
			this->add_path_pos_list(other_province, std::move(pos_list));
		});
	} else {
		territory::process_gsml_scope(scope);
	}
}

void province::initialize()
{
	if (this->get_world()->get_surface_area() != 0 && this->get_area() == 0) {
		this->area = this->get_world()->get_area_per_pixel() * this->pixel_count;
	}

	territory::initialize();
}

void province::initialize_history()
{
	territory::initialize_history();

	if (this->get_county() != nullptr) {
		if (this->has_river()) {
			this->change_population_capacity_additive_modifier(10000); //increase population capacity if this province has a river
		}
		if (this->is_coastal()) {
			this->change_population_capacity_additive_modifier(10000); //increase population capacity if this province is coastal
		}

		this->calculate_trade_node();
	}
}

void province::check() const
{
	if (this->get_terrain() == nullptr) {
		throw std::runtime_error("Province \"" + this->get_identifier() + "\" has no terrain.");
	}

	if (!this->get_color().isValid()) {
		throw std::runtime_error("Province \"" + this->get_identifier() + "\" has no valid color.");
	}

	territory::check();

	if (static_cast<int>(this->get_settlement_holding_slots().size()) > defines::get()->get_max_settlement_slots_per_province()) {
		throw std::runtime_error("Province \"" + this->get_identifier() + "\" has " + std::to_string(this->get_settlement_holding_slots().size()) + " settlement slots, but the maximum settlement slots per province is set to " + std::to_string(defines::get()->get_max_settlement_slots_per_province()) + ".");
	}

	if (static_cast<int>(this->get_palace_holding_slots().size()) > defines::get()->get_max_palace_slots_per_province()) {
		throw std::runtime_error("Province \"" + this->get_identifier() + "\" has " + std::to_string(this->get_palace_holding_slots().size()) + " palace slots, but the maximum palace slots per province is set to " + std::to_string(defines::get()->get_max_palace_slots_per_province()) + ".");
	}

	if (this->get_county() != nullptr) {
		if (this->get_megalopolis() == nullptr) {
			qWarning() << ("Province \"" + this->get_name_qstring() + "\" has no megalopolis holding slot.");
		}
	}
}

void province::check_history() const
{
	/*
	if (this->border_provinces.empty()) {
		throw std::runtime_error("Province \"" + this->get_identifier() + "\" has no border provinces.");
	}
	*/

	if (this->get_trade_node() == nullptr && this->get_owner() != nullptr) {
		throw std::runtime_error("Province \"" + this->get_identifier() + "\" has an owner, but is not assigned to any trade node.");
	}

	try {
		for (const qunique_ptr<wildlife_unit> &wildlife_unit : this->get_wildlife_units()) {
			wildlife_unit->check_history();
		}
	} catch (...) {
		std::throw_with_nested(std::runtime_error("A wildlife unit in province \"" + this->get_identifier() + "\" is in an invalid state."));
	}

	territory::check_history();
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

	gsml_data river_crossings("river_crossings");
	for (const province *province : this->river_crossings) {
		river_crossings.add_value(province->get_identifier());
	}
	cache_data.add_child(std::move(river_crossings));

	for (const holding_slot *slot : this->get_settlement_holding_slots()) {
		gsml_data slot_cache_data = slot->get_cache_data();

		if (!slot_cache_data.is_empty()) {
			cache_data.add_child(std::move(slot_cache_data));
		}
	}

	if (!this->path_pos_map.empty()) {
		gsml_data path_pos_map_data("path_pos_map");
		for (const auto &kv_pair : this->path_pos_map) {
			gsml_data path_pos_list_data(kv_pair.first->get_identifier());
			for (const QPointF &path_pos : kv_pair.second) {
				path_pos_list_data.add_child(gsml_data::from_point(path_pos));
			}
			path_pos_map_data.add_child(std::move(path_pos_list_data));
		}
		cache_data.add_child(std::move(path_pos_map_data));
	}

	return cache_data;
}

void province::do_month()
{
	territory::do_month();

	if (this->get_county() != nullptr) {
		if (this->trade_node_recalculation_needed) {
			this->calculate_trade_node();
			this->set_trade_node_recalculation_needed(false);
		}
	}
}

void province::set_county(landed_title *county)
{
	if (county == this->get_county()) {
		return;
	}

	territory::set_county(county);
	county->set_province(this);
}

void province::set_world(metternich::world *world)
{
	if (world == this->get_world()) {
		return;
	}

	this->world = world;
}

void province::set_megalopolis(holding_slot *megalopolis)
{
	if (megalopolis == this->get_megalopolis()) {
		return;
	}

	if (this->get_megalopolis() != nullptr) {
		this->get_megalopolis()->remove_megalopolis_province(this);
	}

	this->megalopolis = megalopolis;

	if (megalopolis != nullptr) {
		megalopolis->add_megalopolis_province(this);
	}
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
			case map_mode::trade_zone: {
				if (this->get_trading_post_holding() != nullptr) {
					return this->get_trading_post_holding()->get_owner()->get_realm()->get_color();
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

void province::calculate_rect()
{
	QPointF top_left(-1, -1);
	QPointF bottom_right(-1, -1);

	for (const QGeoPolygon &geopolygon : this->get_geopolygons()) {
		for (const QGeoCoordinate &geocoordinate : geopolygon.path()) {
			const QPointF pos = this->get_world()->get_coordinate_posf(geocoordinate);

			if (top_left.x() == -1 || pos.x() < top_left.x()) {
				top_left.setX(pos.x());
			}

			if (top_left.y() == -1 || pos.y() < top_left.y()) {
				top_left.setY(pos.y());
			}

			if (bottom_right.x() == -1 || pos.x() > bottom_right.x()) {
				bottom_right.setX(pos.x());
			}

			if (bottom_right.y() == -1 || pos.y() > bottom_right.y()) {
				bottom_right.setY(pos.y());
			}
		}
	}

	this->rect = QRectF(top_left, bottom_right);
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
		if (old_terrain->get_province_modifier() != nullptr) {
			old_terrain->get_province_modifier()->remove(this);
		}
	}

	this->terrain = terrain;

	if (terrain != nullptr) {
		if (terrain->get_province_modifier() != nullptr) {
			terrain->get_province_modifier()->apply(this);
		}
	}

	emit terrain_changed();
}

void province::calculate_terrain()
{
	std::map<terrain_type *, int> terrain_counts;

	for (holding_slot *slot : this->get_settlement_holding_slots()) {
		if (slot->get_geocoordinate().isValid()) {
			terrain_counts[this->get_world()->get_coordinate_terrain(slot->get_geocoordinate())]++;
		}
	}

	if (terrain_counts.empty()) {
		const QGeoCoordinate &center_geocoordinate = this->get_center_geocoordinate();
		terrain_counts[this->get_world()->get_coordinate_terrain(center_geocoordinate)]++;
	}

	terrain_type *best_terrain_type = nullptr;
	int best_count = 0;
	for (const auto &kv_pair : terrain_counts) {
		terrain_type *terrain_type = kv_pair.first;

		if (terrain_type == nullptr) {
			continue;
		}

		const int count = kv_pair.second;
		if (count > best_count) {
			best_terrain_type = terrain_type;
			best_count = count;
		}
	}

	if (best_terrain_type != nullptr) {
		this->set_terrain(best_terrain_type);
	}
}

void province::set_culture(metternich::culture *culture)
{
	if (culture == this->get_culture()) {
		return;
	}

	const metternich::culture *old_culture = this->get_culture();
	metternich::culture_group *old_culture_group = old_culture ? old_culture->get_culture_group() : nullptr;
	territory::set_culture(culture);
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
	territory::set_religion(religion);
	metternich::religion_group *religion_group = religion ? religion->get_religion_group() : nullptr;

	this->set_trade_node_recalculation_needed(true);

	if (
		map::get()->get_mode() == map_mode::religion
		|| (map::get()->get_mode() == map_mode::religion_group && old_religion_group != religion_group)
	) {
		this->update_color_for_map_mode(map::get()->get_mode());
	}
}

void province::add_holding_slot(holding_slot *holding_slot)
{
	holding_slot->set_province(this);
	territory::add_holding_slot(holding_slot);
}

void province::set_capital_holding_slot(holding_slot *holding_slot)
{
	if (holding_slot == this->get_capital_holding_slot()) {
		return;
	}

	const QPointF &old_main_pos = this->get_main_pos();

	territory::set_capital_holding_slot(holding_slot);

	const QPointF &main_pos = this->get_main_pos();

	if (old_main_pos != main_pos) {
		emit main_pos_changed();
	}
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

	if (notify_engine_interface) {
		engine_interface::get()->emit selected_province_changed();
	}
}

QString province::get_polygons_svg() const
{
	const QRectF bounding_rect = this->get_rect();

	QString svg;

	for (const QGeoPolygon &geopolygon : this->get_geopolygons()) {
		const QList<QGeoCoordinate> &geopolygon_path = geopolygon.path();
		svg += geocoordinate::path_to_svg_string(geopolygon_path, this->get_world()->get_lon_per_pixel(), this->get_world()->get_lat_per_pixel(), bounding_rect);

		for (int i = 0; i < geopolygon.holesCount(); ++i) {
			const QList<QGeoCoordinate> hole_path = geopolygon.holePath(i);
			svg += geocoordinate::path_to_svg_string(hole_path, this->get_world()->get_lon_per_pixel(), this->get_world()->get_lat_per_pixel(), bounding_rect);
		}
	}

	return svg;
}

QVariantList province::get_geopolygons_qvariant_list() const
{
	return container::to_qvariant_list(this->get_geopolygons());
}

QVariantList province::get_geopaths_qvariant_list() const
{
	return container::to_qvariant_list(this->geopaths);
}

void province::calculate_center_pos()
{
	const QGeoShape &main_geoshape = this->get_main_geoshape();
	this->center_geocoordinate = main_geoshape.center();
	this->center_pos = this->get_world()->get_coordinate_posf(this->center_geocoordinate);
}

const QPointF &province::get_main_pos() const
{
	if (this->get_capital_holding_slot() != nullptr && this->get_capital_holding_slot()->get_pos().x() != -1 && this->get_capital_holding_slot()->get_pos().y() != -1) {
		return this->get_capital_holding_slot()->get_pos();
	}

	return this->get_center_pos();
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
