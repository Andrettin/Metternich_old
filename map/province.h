#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"
#include "qunique_ptr.h"
#include "technology/technology_set.h"

#include <QColor>
#include <QGeoCoordinate>
#include <QGeoPath>
#include <QGeoPolygon>
#include <QImage>
#include <QObject>
#include <QRect>
#include <QVariant>

#include <map>
#include <memory>
#include <set>
#include <shared_mutex>
#include <string>
#include <vector>

namespace metternich {

class character;
class culture;
class holding;
class holding_slot;
class holding_type;
class landed_title;
class population_type;
class population_unit;
class region;
class religion;
class technology;
class terrain_type;
class trade_node;
class trade_route;
class wildlife_unit;
class world;
enum class map_mode : int;

class province : public data_entry, public data_type<province>
{
	Q_OBJECT

	Q_PROPERTY(metternich::landed_title* county READ get_county WRITE set_county NOTIFY county_changed)
	Q_PROPERTY(metternich::landed_title* duchy READ get_duchy NOTIFY duchy_changed)
	Q_PROPERTY(metternich::landed_title* de_jure_duchy READ get_de_jure_duchy NOTIFY de_jure_duchy_changed)
	Q_PROPERTY(metternich::landed_title* kingdom READ get_kingdom NOTIFY kingdom_changed)
	Q_PROPERTY(metternich::landed_title* de_jure_kingdom READ get_de_jure_kingdom NOTIFY de_jure_kingdom_changed)
	Q_PROPERTY(metternich::landed_title* empire READ get_empire NOTIFY empire_changed)
	Q_PROPERTY(metternich::landed_title* de_jure_empire READ get_de_jure_empire NOTIFY de_jure_empire_changed)
	Q_PROPERTY(metternich::world* world READ get_world CONSTANT)
	Q_PROPERTY(metternich::trade_node* trade_node READ get_trade_node NOTIFY trade_node_changed)
	Q_PROPERTY(metternich::trade_node* trade_area READ get_trade_area NOTIFY trade_area_changed)
	Q_PROPERTY(bool major_center_of_trade READ is_major_center_of_trade WRITE set_major_center_of_trade NOTIFY major_center_of_trade_changed)
	Q_PROPERTY(QColor color MEMBER color READ get_color)
	Q_PROPERTY(QRect rect READ get_rect CONSTANT)
	Q_PROPERTY(QImage image READ get_image NOTIFY image_changed)
	Q_PROPERTY(metternich::terrain_type* terrain READ get_terrain WRITE set_terrain NOTIFY terrain_changed)
	Q_PROPERTY(metternich::character* owner READ get_owner NOTIFY owner_changed)
	Q_PROPERTY(metternich::culture* culture READ get_culture WRITE set_culture NOTIFY culture_changed)
	Q_PROPERTY(metternich::religion* religion READ get_religion WRITE set_religion NOTIFY religion_changed)
	Q_PROPERTY(int population READ get_population WRITE set_population NOTIFY population_changed)
	Q_PROPERTY(QVariantList wildlife_units READ get_wildlife_units_qvariant_list NOTIFY wildlife_units_changed)
	Q_PROPERTY(QVariantList settlement_holding_slots READ get_settlement_holding_slots_qvariant_list NOTIFY settlement_holding_slots_changed)
	Q_PROPERTY(QVariantList settlement_holdings READ get_settlement_holdings_qvariant_list NOTIFY settlement_holdings_changed)
	Q_PROPERTY(metternich::holding_slot* capital_holding_slot READ get_capital_holding_slot WRITE set_capital_holding_slot NOTIFY capital_holding_slot_changed)
	Q_PROPERTY(metternich::holding* capital_holding READ get_capital_holding WRITE set_capital_holding)
	Q_PROPERTY(QVariantList palace_holding_slots READ get_palace_holding_slots_qvariant_list CONSTANT)
	Q_PROPERTY(metternich::holding_slot* fort_holding_slot READ get_fort_holding_slot CONSTANT)
	Q_PROPERTY(metternich::holding_slot* university_holding_slot READ get_university_holding_slot CONSTANT)
	Q_PROPERTY(metternich::holding_slot* hospital_holding_slot READ get_hospital_holding_slot CONSTANT)
	Q_PROPERTY(metternich::holding_slot* trading_post_holding_slot READ get_trading_post_holding_slot NOTIFY trading_post_holding_slot_changed)
	Q_PROPERTY(metternich::holding_slot* factory_holding_slot READ get_factory_holding_slot CONSTANT)
	Q_PROPERTY(QVariantList technologies READ get_technologies_qvariant_list NOTIFY technologies_changed)
	Q_PROPERTY(bool selected READ is_selected WRITE set_selected NOTIFY selected_changed)
	Q_PROPERTY(bool selectable READ is_selectable CONSTANT)
	Q_PROPERTY(QGeoCoordinate center_coordinate READ get_center_coordinate CONSTANT)
	Q_PROPERTY(QPoint center_pos READ get_center_pos CONSTANT)
	Q_PROPERTY(QVariantList geopolygons READ get_geopolygons_qvariant_list CONSTANT)
	Q_PROPERTY(QVariantList geopaths READ get_geopaths_qvariant_list CONSTANT)
	Q_PROPERTY(bool always_write_geodata MEMBER always_write_geodata READ always_writes_geodata)
	Q_PROPERTY(bool write_geojson MEMBER write_geojson_value READ writes_geojson)

public:
	static constexpr const char *class_identifier = "province";
	static constexpr const char *database_folder = "provinces";
	static constexpr const char *prefix = "p_";
	static constexpr QRgb empty_rgb = qRgb(0, 0, 0);
	static const inline QColor water_province_color = QColor("#4682b4"); //steel blue
	static const inline QColor wasteland_province_color = QColor(Qt::darkGray);
	static const inline QColor empty_province_color = QColor("#f5f5dc");
	static constexpr int base_distance = 100; //in kilometers; distance used as the base unit for e.g. the trade cost modifier per distance

	static std::set<std::string> get_database_dependencies();
	static province *add(const std::string &identifier);
	static province *get_by_rgb(const QRgb &rgb, const bool should_find = true);

	static province *get_selected_province()
	{
		return province::selected_province;
	}

private:
	static inline std::map<QRgb, province *> instances_by_rgb;
	static inline province *selected_province = nullptr;

public:
	province(const std::string &identifier);
	virtual ~province() override;

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void process_gsml_dated_property(const gsml_property &property, const QDateTime &date) override;
	virtual void process_gsml_dated_scope(const gsml_data &scope, const QDateTime &date) override;
	virtual void initialize() override;
	virtual void initialize_history() override;
	virtual void check() const override;
	virtual void check_history() const override;
	virtual gsml_data get_cache_data() const override;

	void do_day();
	void do_month();

	std::string get_identifier_without_prefix() const
	{
		const size_t prefix_size = std::string(province::prefix).size();
		return this->get_identifier().substr(prefix_size, this->get_identifier().size() - prefix_size);
	}

	virtual std::string get_name() const override;

	std::vector<std::vector<std::string>> get_tag_suffix_list_with_fallbacks() const;

	landed_title *get_county() const
	{
		return this->county;
	}

	void set_county(landed_title *county);

	landed_title *get_duchy() const;
	landed_title *get_de_jure_duchy() const;
	landed_title *get_kingdom() const;
	landed_title *get_de_jure_kingdom() const;
	landed_title *get_empire() const;
	landed_title *get_de_jure_empire() const;

	world *get_world() const
	{
		return this->world;
	}

	void set_world(world *world);

	trade_node *get_trade_node() const
	{
		return this->trade_node;
	}

	void set_trade_node(trade_node *trade_node);
	void calculate_trade_node();
	trade_node *get_best_trade_node_from_list(const std::set<trade_node *> &trade_nodes) const;

	trade_node *get_trade_area() const;

	const QColor &get_color() const
	{
		return this->color;
	}

	const QColor &get_map_mode_color() const
	{
		return this->map_mode_color;
	}

	void set_map_mode_color(const QColor &color)
	{
		if (color == this->get_map_mode_color()) {
			return;
		}

		this->map_mode_color = color;
		this->update_image();
	}

	const QColor &get_color_for_map_mode(const map_mode mode) const;

	void update_color_for_map_mode(const map_mode mode)
	{
		const QColor &color = this->get_color_for_map_mode(mode);
		this->set_map_mode_color(color);
	}

	const QRect &get_rect() const
	{
		return this->rect;
	}

	void create_image(const std::vector<int> &pixel_indexes);
	void set_border_pixels(const std::vector<int> &pixel_indexes);
	void update_image();
	void write_geodata_to_image(QImage &image, QImage &terrain_image) const;
	void write_geopath_endpoints_to_image(QImage &image, QImage &terrain_image) const;
	void write_geoshape_to_image(QImage &image, const QGeoShape &geoshape, QImage &terrain_image) const;
	void write_geojson() const;

	const QImage &get_image() const
	{
		return this->image;
	}

	terrain_type *get_terrain() const
	{
		return this->terrain;
	}

	void set_terrain(terrain_type *terrain);

	character *get_owner() const;

	metternich::culture *get_culture() const
	{
		return this->culture;
	}

	void set_culture(culture *culture);

	metternich::religion *get_religion() const
	{
		return this->religion;
	}

	void set_religion(religion *religion);

	int get_area() const
	{
		return this->area;
	}

	int get_population() const
	{
		return this->population;
	}

	void set_population(const int population);

	void change_population(const int change)
	{
		this->set_population(this->get_population() + change);
	}

	void calculate_population();

	int get_population_capacity_additive_modifier() const
	{
		return this->population_capacity_additive_modifier;
	}

	void set_population_capacity_additive_modifier(const int population_capacity_modifier);

	void change_population_capacity_additive_modifier(const int change)
	{
		this->set_population_capacity_additive_modifier(this->get_population_capacity_additive_modifier() + change);
	}

	int get_population_capacity_modifier() const
	{
		return this->population_capacity_modifier;
	}

	void set_population_capacity_modifier(const int population_capacity_modifier);

	void change_population_capacity_modifier(const int change)
	{
		this->set_population_capacity_modifier(this->get_population_capacity_modifier() + change);
	}

	int get_population_growth_modifier() const
	{
		return this->population_growth_modifier;
	}

	void set_population_growth_modifier(const int population_capacity_modifier);

	void change_population_growth_modifier(const int change)
	{
		this->set_population_growth_modifier(this->get_population_growth_modifier() + change);
	}

	void calculate_population_groups();

	holding_slot *get_holding_slot(const std::string &holding_slot_str) const;
	void add_holding_slot(holding_slot *holding_slot);

	const std::vector<holding_slot *> &get_settlement_holding_slots() const
	{
		return this->settlement_holding_slots;
	}

	QVariantList get_settlement_holding_slots_qvariant_list() const;

	const std::vector<holding *> &get_settlement_holdings() const
	{
		return this->settlement_holdings;
	}

	QVariantList get_settlement_holdings_qvariant_list() const;

	void create_holding(holding_slot *holding_slot, holding_type *type);
	void destroy_holding(holding_slot *holding_slot);

	holding_slot *get_capital_holding_slot() const
	{
		return this->capital_holding_slot;
	}

	void set_capital_holding_slot(holding_slot *holding_slot);

	holding *get_capital_holding() const;
	void set_capital_holding(holding *holding);

	const std::vector<holding_slot *> &get_palace_holding_slots() const
	{
		return this->palace_holding_slots;
	}

	QVariantList get_palace_holding_slots_qvariant_list() const;

	holding_slot *get_fort_holding_slot() const
	{
		return this->fort_holding_slot;
	}

	holding_slot *get_university_holding_slot() const
	{
		return this->university_holding_slot;
	}

	holding_slot *get_hospital_holding_slot() const
	{
		return this->hospital_holding_slot;
	}

	holding_slot *get_trading_post_holding_slot() const
	{
		return this->trading_post_holding_slot;
	}

	std::string get_trading_post_holding_slot_identifier() const;
	void create_trading_post_holding_slot();
	void destroy_trading_post_holding_slot();

	holding_slot *get_factory_holding_slot() const
	{
		return this->factory_holding_slot;
	}

	const std::set<region *> &get_regions() const
	{
		return this->regions;
	}

	void add_region(region *region)
	{
		this->regions.insert(region);
	}

	void remove_region(region *region)
	{
		this->regions.erase(region);
	}

	bool is_in_region(region *region) const
	{
		return this->regions.contains(region);
	}

	const std::set<province *> &get_border_provinces() const
	{
		return this->border_provinces;
	}

	void add_border_province(province *province)
	{
		this->border_provinces.insert(province);
	}

	bool borders_water() const;
	bool borders_river() const;
	bool has_river() const;

	bool has_inner_river() const
	{
		return this->inner_river;
	}

	void set_inner_river(const bool inner_river)
	{
		if (this->has_inner_river() == inner_river) {
			return;
		}

		this->inner_river = inner_river;
	}

	bool is_coastal() const;
	bool is_water() const;
	bool is_ocean() const;
	bool is_river() const;

	bool can_have_trading_post() const
	{
		return this->is_coastal() || this->has_any_trade_route();
	}

	const technology_set &get_technologies() const
	{
		return this->technologies;
	}

	QVariantList get_technologies_qvariant_list() const;

	bool has_technology(technology *technology) const
	{
		return this->technologies.contains(technology);
	}

	Q_INVOKABLE void add_technology(technology *technology);
	Q_INVOKABLE void remove_technology(technology *technology);

	bool is_center_of_trade() const;

	bool is_major_center_of_trade() const
	{
		return this->major_center_of_trade;
	}

	void set_major_center_of_trade(const bool major_center_of_trade);

	bool has_any_trade_route() const
	{
		return !this->trade_routes.empty();
	}

	bool has_trade_route(trade_route *route) const
	{
		return this->trade_routes.contains(route);
	}

	void add_trade_route(trade_route *route);
	void remove_trade_route(trade_route *route);

	long long int get_meters_distance_to(const province *other_province) const
	{
		return static_cast<long long int>(this->get_center_coordinate().distanceTo(other_province->get_center_coordinate()));
	}

	int get_kilometers_distance_to(const province *other_province) const
	{
		return static_cast<int>(this->get_meters_distance_to(other_province) / 1000);
	}

	bool is_selected() const
	{
		return this->selected;
	}

	void set_selected(const bool selected, const bool notify_engine_interface = true);
	bool is_selectable() const;

	const std::vector<qunique_ptr<population_unit>> &get_population_units() const
	{
		return this->population_units;
	}

	void add_population_unit(qunique_ptr<population_unit> &&population_unit);

	Q_INVOKABLE QVariantList get_population_per_type_qvariant_list() const;
	Q_INVOKABLE QVariantList get_population_per_culture_qvariant_list() const;
	Q_INVOKABLE QVariantList get_population_per_religion_qvariant_list() const;

	const std::vector<qunique_ptr<wildlife_unit>> &get_wildlife_units() const
	{
		return this->wildlife_units;
	}

	void add_wildlife_unit(qunique_ptr<wildlife_unit> &&wildlife_unit);
	QVariantList get_wildlife_units_qvariant_list() const;
	void sort_wildlife_units();
	void remove_empty_wildlife_units();

	const std::vector<QGeoPolygon> &get_geopolygons() const
	{
		return this->geopolygons;
	}

	QVariantList get_geopolygons_qvariant_list() const;
	QVariantList get_geopaths_qvariant_list() const;

	bool contains_coordinate(const QGeoCoordinate &coordinate) const
	{
		//get whether a coordinate is located in the province
		for (const QGeoPolygon &geopolygon : this->geopolygons) {
			if (geopolygon.contains(coordinate)) {
				return true;
			}
		}

		for (const QGeoPath &geopath : this->geopaths) {
			if (geopath.contains(coordinate)) {
				return true;
			}
		}

		return false;
	}

	QGeoCoordinate get_center_coordinate() const;

	QPoint get_center_pos() const
	{
		return this->center_pos;
	}

	bool always_writes_geodata() const
	{
		return this->always_write_geodata;
	}

	bool writes_geojson() const
	{
		return this->write_geojson_value;
	}

	void set_trade_node_recalculation_needed(const bool recalculation_needed, const bool recalculate_for_dependent_provinces = true);

signals:
	void county_changed();
	void duchy_changed();
	void de_jure_duchy_changed();
	void kingdom_changed();
	void de_jure_kingdom_changed();
	void empire_changed();
	void de_jure_empire_changed();
	void trade_node_changed();
	void trade_area_changed();
	void major_center_of_trade_changed();
	void image_changed();
	void terrain_changed();
	void owner_changed();
	void culture_changed();
	void religion_changed();
	void population_changed();
	void population_groups_changed();
	void wildlife_units_changed();
	void settlement_holding_slots_changed();
	void settlement_holdings_changed();
	void capital_holding_slot_changed();
	void trading_post_holding_slot_changed();
	void technologies_changed();
	void trade_routes_changed();
	void selected_changed();

private:
	landed_title *county = nullptr;
	world *world = nullptr;
	trade_node *trade_node = nullptr;
	QColor color; //the color used to identify the province in the province map
	QColor map_mode_color; //the color used for the province by the current map mode
	QRect rect; //the rectangle that the province occupies
	QPoint center_pos;
	QImage image; //the province's image to be drawn on-screen
	terrain_type *terrain = nullptr;
	metternich::culture *culture = nullptr;
	metternich::religion *religion = nullptr;
	int pixel_count = 0; //the amount of pixels that the province takes on the map
	int area = 0; //the area of the province, in square kilometers; used to calculate holding sizes
	int population = 0; //the sum of the population of all of the province's settlement holdings
	int population_capacity_additive_modifier = 0; //the population capacity additive modifier which the province provides to its holdings
	int population_capacity_modifier = 0; //the population capacity modifier which the province provides to its holdings
	int population_growth_modifier = 0; //the population growth modifier which the province provides to its holdings
	std::vector<holding_slot *> settlement_holding_slots;
	std::vector<holding *> settlement_holdings;
	holding_slot *capital_holding_slot = nullptr;
	std::vector<holding_slot *> palace_holding_slots;
	holding_slot *fort_holding_slot = nullptr;
	holding_slot *university_holding_slot = nullptr;
	holding_slot *hospital_holding_slot = nullptr;
	holding_slot *trading_post_holding_slot = nullptr;
	holding_slot *factory_holding_slot = nullptr;
	std::set<region *> regions; //the regions to which this province belongs
	std::set<province *> border_provinces; //provinces bordering this one
	technology_set technologies; //the technologies acquired for the province
	std::set<trade_route *> trade_routes; //the trade routes going through the province
	bool major_center_of_trade = false;
	std::vector<qunique_ptr<population_unit>> population_units; //population units set for this province in history, used during initialization to generate population units in the province's settlements
	std::map<population_type *, int> population_per_type; //the population for each population type
	std::map<metternich::culture *, int> population_per_culture; //the population for each culture
	std::map<metternich::religion *, int> population_per_religion; //the population for each religion
	mutable std::shared_mutex population_groups_mutex;
	std::vector<qunique_ptr<wildlife_unit>> wildlife_units; //wildlife units set for this province in history
	std::vector<QGeoPolygon> geopolygons;
	std::vector<QGeoPath> geopaths;
	bool inner_river = false; //whether the province has a minor river flowing through it
	bool selected = false;
	bool always_write_geodata = false;
	bool write_geojson_value = false;
	bool trade_node_recalculation_needed = false;
};

}
