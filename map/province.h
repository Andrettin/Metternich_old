#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

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
#include <string>
#include <vector>

namespace metternich {

class csv_data;
class culture;
class holding;
class holding_slot;
class holding_type;
class landed_title;
class population_type;
class population_unit;
class region;
class religion;
class terrain_type;
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
	Q_PROPERTY(QColor color READ get_color CONSTANT)
	Q_PROPERTY(QRect rect READ get_rect CONSTANT)
	Q_PROPERTY(QImage image READ get_image NOTIFY image_changed)
	Q_PROPERTY(metternich::terrain_type* terrain READ get_terrain WRITE set_terrain NOTIFY terrain_changed)
	Q_PROPERTY(metternich::culture* culture READ get_culture WRITE set_culture NOTIFY culture_changed)
	Q_PROPERTY(metternich::religion* religion READ get_religion WRITE set_religion NOTIFY religion_changed)
	Q_PROPERTY(int population READ get_population WRITE set_population NOTIFY population_changed)
	Q_PROPERTY(QVariantList settlement_holding_slots READ get_settlement_holding_slots_qvariant_list NOTIFY settlement_holding_slots_changed)
	Q_PROPERTY(QVariantList settlement_holdings READ get_settlement_holdings_qvariant_list NOTIFY settlement_holdings_changed)
	Q_PROPERTY(metternich::holding* capital_holding READ get_capital_holding WRITE set_capital_holding NOTIFY capital_holding_changed)
	Q_PROPERTY(QVariantList palace_holding_slots READ get_palace_holding_slots_qvariant_list CONSTANT)
	Q_PROPERTY(metternich::holding_slot* fort_holding_slot READ get_fort_holding_slot CONSTANT)
	Q_PROPERTY(metternich::holding_slot* university_holding_slot READ get_university_holding_slot CONSTANT)
	Q_PROPERTY(metternich::holding_slot* hospital_holding_slot READ get_hospital_holding_slot CONSTANT)
	Q_PROPERTY(bool selected READ is_selected WRITE set_selected NOTIFY selected_changed)
	Q_PROPERTY(bool selectable READ is_selectable CONSTANT)
	Q_PROPERTY(QGeoCoordinate center_coordinate READ get_center_coordinate CONSTANT)
	Q_PROPERTY(QVariantList geopolygons READ get_geopolygons_qvariant_list CONSTANT)
	Q_PROPERTY(QVariantList geopaths READ get_geopaths_qvariant_list CONSTANT)
	Q_PROPERTY(bool always_write_geodata MEMBER always_write_geodata READ always_writes_geodata)
	Q_PROPERTY(bool write_geojson MEMBER write_geojson_value READ writes_geojson)

public:
	static constexpr const char *class_identifier = "province";
	static constexpr const char *database_folder = "provinces";
	static constexpr const char *prefix = "p_";
	static constexpr QRgb empty_rgb = qRgb(0, 0, 0);
	static const inline QColor water_province_color = QColor("#0080ff");
	static const inline QColor wasteland_province_color = QColor(Qt::darkGray);

	static std::set<std::string> get_database_dependencies();
	static province *add(const std::string &identifier);
	static province *get_by_rgb(const QRgb &rgb, const bool should_find = true);

	static const std::set<province *> &get_river_provinces()
	{
		return province::river_provinces;
	}

	static province *get_selected_province()
	{
		return province::selected_province;
	}

private:
	static inline std::map<QRgb, province *> instances_by_rgb;
	static inline std::set<province *> river_provinces;
	static inline province *selected_province = nullptr;

public:
	province(const std::string &identifier);
	virtual ~province() override;

	virtual void process_gsml_property(const gsml_property &property) override;
	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void process_gsml_dated_scope(const gsml_data &scope, const QDateTime &date) override;
	virtual void initialize() override;
	virtual void initialize_history() override;
	virtual void check() const override;
	virtual gsml_data get_cache_data() const override;

	void do_day();
	void do_month();

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

	const QColor &get_color() const
	{
		return this->color;
	}

	const QColor &get_map_mode_color(map_mode mode) const;

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

	holding *get_capital_holding() const
	{
		return this->capital_holding;
	}

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

	const std::vector<region *> &get_regions() const
	{
		return this->regions;
	}

	void add_region(region *region)
	{
		this->regions.push_back(region);
	}

	void remove_region(region *region)
	{
		this->regions.erase(std::remove(this->regions.begin(), this->regions.end(), region), this->regions.end());
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

	bool is_selected() const
	{
		return this->selected;
	}

	void set_selected(const bool selected, const bool notify_engine_interface = true);
	bool is_selectable() const;

	const std::vector<std::unique_ptr<population_unit>> &get_population_units() const
	{
		return this->population_units;
	}

	void add_population_unit(std::unique_ptr<population_unit> &&population_unit);

	Q_INVOKABLE QVariantList get_population_per_type_qvariant_list() const;
	Q_INVOKABLE QVariantList get_population_per_culture_qvariant_list() const;
	Q_INVOKABLE QVariantList get_population_per_religion_qvariant_list() const;

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

	bool always_writes_geodata() const
	{
		return this->always_write_geodata;
	}

	bool writes_geojson() const
	{
		return this->write_geojson_value;
	}

signals:
	void county_changed();
	void duchy_changed();
	void de_jure_duchy_changed();
	void kingdom_changed();
	void de_jure_kingdom_changed();
	void empire_changed();
	void de_jure_empire_changed();
	void image_changed();
	void terrain_changed();
	void culture_changed();
	void religion_changed();
	void population_changed();
	void population_groups_changed();
	void settlement_holding_slots_changed();
	void settlement_holdings_changed();
	void capital_holding_changed();
	void selected_changed();

private:
	landed_title *county = nullptr;
	QColor color; //the color used to identify the province in the province map
	QRect rect; //the rectangle that the province occupies
	QImage image; //the province's image to be drawn on-screen
	terrain_type *terrain = nullptr;
	metternich::culture *culture = nullptr;
	metternich::religion *religion = nullptr;
	int population = 0; //the sum of the population of all of the province's settlement holdings
	int population_capacity_additive_modifier = 0; //the population capacity additive modifier which the province provides to its holdings
	int population_capacity_modifier = 0; //the population capacity modifier which the province provides to its holdings
	int population_growth_modifier = 0; //the population growth modifier which the province provides to its holdings
	std::vector<holding_slot *> settlement_holding_slots;
	std::vector<holding *> settlement_holdings;
	holding *capital_holding = nullptr;
	std::vector<holding_slot *> palace_holding_slots;
	holding_slot *fort_holding_slot = nullptr;
	holding_slot *university_holding_slot = nullptr;
	holding_slot *hospital_holding_slot = nullptr;
	std::vector<region *> regions; //the regions to which this province belongs
	std::set<province *> border_provinces; //provinces bordering this one
	bool selected = false;
	std::vector<std::unique_ptr<population_unit>> population_units; //population units set for this province in history, used during initialization to generate population units in the province's settlements
	std::map<population_type *, int> population_per_type; //the population for each population type
	std::map<metternich::culture *, int> population_per_culture; //the population for each culture
	std::map<metternich::religion *, int> population_per_religion; //the population for each religion
	std::vector<QGeoPolygon> geopolygons;
	std::vector<QGeoPath> geopaths;
	bool inner_river = false; //whether the province has a minor river flowing through it
	bool always_write_geodata = false;
	bool write_geojson_value = false;
};

}
