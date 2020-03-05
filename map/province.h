#pragma once

#include "database/data_type.h"
#include "map/territory.h"
#include "qunique_ptr.h"

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

class population_unit;
class terrain_type;
class trade_node;
class trade_route;
class wildlife_unit;
class world;
enum class map_mode;

class province final : public territory, public data_type<province>
{
	Q_OBJECT

	Q_PROPERTY(metternich::world* world READ get_world CONSTANT)
	Q_PROPERTY(metternich::holding_slot* megalopolis READ get_megalopolis WRITE set_megalopolis)
	Q_PROPERTY(metternich::trade_node* trade_node READ get_trade_node NOTIFY trade_node_changed)
	Q_PROPERTY(int trade_node_trade_cost READ get_trade_node_trade_cost NOTIFY trade_node_trade_cost_changed)
	Q_PROPERTY(QColor color MEMBER color READ get_color)
	Q_PROPERTY(QRect rect READ get_rect CONSTANT)
	Q_PROPERTY(QImage image READ get_image NOTIFY image_changed)
	Q_PROPERTY(metternich::terrain_type* terrain READ get_terrain WRITE set_terrain NOTIFY terrain_changed)
	Q_PROPERTY(QVariantList wildlife_units READ get_wildlife_units_qvariant_list NOTIFY wildlife_units_changed)
	Q_PROPERTY(bool selected READ is_selected WRITE set_selected NOTIFY selected_changed)
	Q_PROPERTY(QGeoCoordinate center_coordinate READ get_center_coordinate CONSTANT)
	Q_PROPERTY(QPoint center_pos READ get_center_pos CONSTANT)
	Q_PROPERTY(QPoint main_pos READ get_main_pos NOTIFY main_pos_changed)
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
	virtual void initialize() override;
	virtual void initialize_history() override;
	virtual void check() const override;
	virtual void check_history() const override;
	virtual gsml_data get_cache_data() const override;

	void do_day();
	virtual void do_month() override final;

	virtual std::string get_identifier_without_prefix() const override
	{
		const size_t prefix_size = std::string(province::prefix).size();
		return this->get_identifier().substr(prefix_size, this->get_identifier().size() - prefix_size);
	}

	virtual void set_county(landed_title *county) override;

	world *get_world() const
	{
		return this->world;
	}

	void set_world(world *world);

	holding_slot *get_megalopolis() const
	{
		return this->megalopolis;
	}

	void set_megalopolis(holding_slot *megalopolis);

	trade_node *get_trade_node() const
	{
		return this->trade_node;
	}

	void set_trade_node(trade_node *trade_node);
	void calculate_trade_node();
	std::pair<trade_node *, int> get_best_trade_node_from_list(const std::set<trade_node *> &trade_nodes) const;

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

	virtual void set_culture(metternich::culture *culture) override;
	virtual void set_religion(metternich::religion *religion) override;

	int get_area() const
	{
		return this->area;
	}

	virtual void add_holding_slot(holding_slot *holding_slot) override;

	virtual void set_capital_holding_slot(holding_slot *holding_slot) override;

	const std::set<province *> &get_border_provinces() const
	{
		return this->border_provinces;
	}

	bool borders_province(province *other_province) const
	{
		return this->border_provinces.contains(other_province);
	}

	void add_border_province(province *province)
	{
		this->border_provinces.insert(province);
	}

	bool borders_water() const;
	bool borders_river() const;

	bool has_river_crossing_with(province *other_province) const
	{
		return this->river_crossings.contains(other_province);
	}

	void add_river_crossing(province *other_province)
	{
		this->river_crossings.insert(other_province);
		this->add_border_province(other_province);
	}

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

	bool is_land() const
	{
		return !this->is_water();
	}

	virtual bool can_have_trading_post() const override
	{
		if (!territory::can_have_trading_post()) {
			return false;
		}

		return this->is_coastal() || this->has_any_active_trade_route();
	}

	bool is_center_of_trade() const;

	int get_trade_node_trade_cost() const
	{
		return this->trade_node_trade_cost;
	}

	void set_trade_node_trade_cost(const int trade_cost)
	{
		if (trade_cost == this->get_trade_node_trade_cost()) {
			return;
		}

		if (trade_cost < 0) {
			throw std::runtime_error("Tried to set a negative trade node trade cost for province \"" + this->get_identifier() + "\".");
		}

		this->trade_node_trade_cost = trade_cost;
		emit trade_node_trade_cost_changed();
	}

	const std::set<trade_route *> &get_trade_routes() const
	{
		return this->trade_routes;
	}

	bool has_any_trade_route() const
	{
		return !this->trade_routes.empty();
	}

	bool has_trade_route(trade_route *route) const
	{
		return this->trade_routes.contains(route);
	}

	void add_trade_route(trade_route *route)
	{
		this->trade_routes.insert(route);
	}

	void remove_trade_route(trade_route *route)
	{
		this->trade_routes.erase(route);
	}

	bool has_any_active_trade_route() const
	{
		return !this->active_trade_routes.empty();
	}

	void add_active_trade_route(trade_route *route);
	void remove_active_trade_route(trade_route *route);
	bool has_trade_route_connection_to(const province *other_province) const;
	bool has_any_trade_route_land_connection() const;

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

	const std::vector<QGeoPath> &get_geopaths() const
	{
		return this->geopaths;
	}

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

	const QPoint &get_center_pos() const
	{
		return this->center_pos;
	}

	const QPoint &get_main_pos() const;

	bool is_pos_valid(const QPoint &pos) const
	{
		//whether the position is a valid one in the province
		return this->rect.contains(pos) && this->image.pixel(pos - this->rect.topLeft()) != qRgba(0, 0, 0, 0);
	}

	QPoint get_nearest_valid_pos(const QPoint &pos) const;

	const std::vector<QPoint> &get_path_pos_list(const province *other_province) const
	{
		static std::vector<QPoint> empty_list;

		auto find_iterator = this->path_pos_map.find(other_province);
		if (find_iterator != this->path_pos_map.end()) {
			return find_iterator->second;
		}

		return empty_list;
	}

	void add_path_pos_list(const province *other_province, std::vector<QPoint> &&path_pos_list)
	{
		this->path_pos_map[other_province] = std::move(path_pos_list);
	}

	bool is_line_to_valid(const QPoint &start_pos, const QPoint &target_pos, const province *other_province) const
	{
		//check if every point on a line from the given position to the target point is on either province
		const QPoint diff(std::abs(start_pos.x() - target_pos.x()), std::abs(start_pos.y() - target_pos.y()));
		const QPoint multiplier(start_pos.x() < target_pos.x() ? 1 : -1, start_pos.y() < target_pos.y() ? 1 : -1);
		if (diff.x() >= diff.y()) {
			const int y_modulo = diff.y() != 0 ? diff.x() / diff.y() : 0;

			QPoint offset(1, 0);
			for (; offset.x() <= diff.x(); offset.rx()++) {
				if (y_modulo != 0 && offset.x() % y_modulo == 0) {
					offset.ry()++;
				}

				const QPoint line_pos = start_pos + QPoint(offset.x() * multiplier.x(), offset.y() * multiplier.y());
				if (!this->is_pos_valid(line_pos) && !other_province->is_pos_valid(line_pos)) {
					return false;
				}
			}
		} else {
			const int x_modulo = diff.x() != 0 ? diff.y() / diff.x() : 0;

			QPoint offset(0, 1);
			for (; offset.y() <= diff.y(); offset.ry()++) {
				if (x_modulo != 0 && offset.y() % x_modulo == 0) {
					offset.rx()++;
				}

				const QPoint line_pos = start_pos + QPoint(offset.x() * multiplier.x(), offset.y() * multiplier.y());
				if (!this->is_pos_valid(line_pos) && !other_province->is_pos_valid(line_pos)) {
					return false;
				}
			}
		}

		return true;
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
	void trade_node_changed();
	void trade_node_trade_cost_changed();
	void image_changed();
	void terrain_changed();
	void wildlife_units_changed();
	void main_pos_changed();
	void active_trade_routes_changed();
	void selected_changed();

private:
	world *world = nullptr;
	holding_slot *megalopolis = nullptr;
	trade_node *trade_node = nullptr;
	QColor color; //the color used to identify the province in the province map
	QColor map_mode_color; //the color used for the province by the current map mode
	QRect rect; //the rectangle that the province occupies
	QPoint center_pos;
	QImage image; //the province's image to be drawn on-screen
	terrain_type *terrain = nullptr;
	int pixel_count = 0; //the amount of pixels that the province takes on the map
	int area = 0; //the area of the province, in square kilometers; used to calculate holding sizes
	std::set<province *> border_provinces; //provinces bordering this one
	std::set<province *> river_crossings; //provinces bordering this one which are reached by crossing a major river
	std::set<trade_route *> trade_routes; //the trade routes going through the province
	std::set<trade_route *> active_trade_routes; //the active trade routes going through the province
	int trade_node_trade_cost = 0;
	std::vector<qunique_ptr<wildlife_unit>> wildlife_units; //wildlife units set for this province in history
	std::map<const province *, std::vector<QPoint>> path_pos_map; //lists of the path positions for the paths between this province and its border provinces
	std::vector<QGeoPolygon> geopolygons;
	std::vector<QGeoPath> geopaths;
	bool inner_river = false; //whether the province has a minor river flowing through it
	bool selected = false;
	bool always_write_geodata = false;
	bool write_geojson_value = false;
	bool trade_node_recalculation_needed = false;
};

}
