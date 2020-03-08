#pragma once

#include "database/data_type.h"
#include "map/territory.h"

#include <QGeoCoordinate>
#include <QPointF>

#include <filesystem>
#include <set>
#include <string_view>

namespace metternich {

class landed_title;
class pathfinder;
class province;
class star_system;
class terrain_type;
class trade_node;
class trade_route;
class world_type;

class world final : public territory, public data_type<world>
{
	Q_OBJECT

	Q_PROPERTY(metternich::world_type* type READ get_type WRITE set_type NOTIFY type_changed)
	Q_PROPERTY(metternich::star_system* star_system READ get_star_system WRITE set_star_system NOTIFY star_system_changed)
	Q_PROPERTY(QString texture_path READ get_texture_path_qstring CONSTANT)
	Q_PROPERTY(QGeoCoordinate astrocoordinate READ get_astrocoordinate CONSTANT)
	Q_PROPERTY(int astrodistance READ get_astrodistance WRITE set_astrodistance NOTIFY astrodistance_changed)
	Q_PROPERTY(int astrodistance_pc READ get_astrodistance_pc WRITE set_astrodistance_pc NOTIFY astrodistance_changed)
	Q_PROPERTY(QPointF orbit_position READ get_orbit_position NOTIFY orbit_position_changed)
	Q_PROPERTY(metternich::world* orbit_center READ get_orbit_center WRITE set_orbit_center NOTIFY orbit_center_changed)
	Q_PROPERTY(double distance_from_orbit_center READ get_distance_from_orbit_center WRITE set_distance_from_orbit_center NOTIFY distance_from_orbit_center_changed)
	Q_PROPERTY(double distance_from_orbit_center_au READ get_distance_from_orbit_center_au WRITE set_distance_from_orbit_center_au NOTIFY distance_from_orbit_center_changed)
	Q_PROPERTY(double rotation READ get_rotation WRITE set_rotation NOTIFY rotation_changed)
	Q_PROPERTY(QPointF cosmic_map_pos READ get_cosmic_map_pos NOTIFY cosmic_map_pos_changed)
	Q_PROPERTY(double cosmic_size READ get_cosmic_size CONSTANT)
	Q_PROPERTY(bool star READ is_star CONSTANT)
	Q_PROPERTY(bool planet READ is_planet CONSTANT)
	Q_PROPERTY(bool moon READ is_moon CONSTANT)
	Q_PROPERTY(bool map READ has_map WRITE set_map)
	Q_PROPERTY(bool map_active MEMBER map_active READ is_map_active)
	Q_PROPERTY(QSize map_size READ get_map_size CONSTANT)
	Q_PROPERTY(int surface_area MEMBER surface_area READ get_surface_area)
	Q_PROPERTY(int radius MEMBER radius READ get_radius)
	Q_PROPERTY(int solar_radius READ get_solar_radius WRITE set_solar_radius)
	Q_PROPERTY(int jovian_radius READ get_jovian_radius WRITE set_jovian_radius)
	Q_PROPERTY(QVariantList provinces READ get_provinces_qvariant_list CONSTANT)
	Q_PROPERTY(QVariantList trade_nodes READ get_trade_nodes_qvariant_list CONSTANT)
	Q_PROPERTY(QVariantList trade_routes READ get_trade_routes_qvariant_list CONSTANT)
	Q_PROPERTY(QString cache_path READ get_cache_path_qstring CONSTANT)

public:
	static constexpr const char *class_identifier = "world";
	static constexpr const char *database_folder = "worlds";
	static constexpr double default_moon_size = 16;
	static constexpr double default_planet_size = 32;
	static constexpr double default_star_size = 128;
	static constexpr int solar_radius = 695700; //in kilometers
	static constexpr int jovian_radius = 69911; //in kilometers
	static constexpr int solar_absolute_magnitude = 483; //4.83
	static constexpr int million_km_per_pixel = 1;
	static constexpr int light_years_per_hundred_parsecs = 326; //1 parsec = 3.26 light years
	static constexpr int min_orbit_distance = 32; //minimum distance between an orbit and the next one in the system
	static constexpr int max_orbit_distance = 64; //maximum distance between an orbit and the next one in the system
	static constexpr int astrodistance_multiplier = 1024;
	static constexpr int million_km_per_au = 150;
	static constexpr bool revolution_enabled = false;

	static std::set<std::string> get_database_dependencies();

	static const std::vector<world *> &get_map_worlds()
	{
		return world::map_worlds;
	}

	static void process_geojson_feature(const QVariantMap &feature)
	{
		const QVariantMap properties = feature.value("properties").toMap();
		const QString world_identifier = properties.value("world").toString();

		world *world = world::get(world_identifier.toStdString());

		const QGeoCircle geocircle = feature.value("data").value<QGeoCircle>();
		world->set_astrocoordinate(geocircle.center());
	}

private:
	static inline std::vector<world *> map_worlds;

public:
	world(const std::string &identifier);
	virtual ~world() override;

	virtual void initialize() override;

	virtual void check() const override
	{
		if (this->get_type() == nullptr) {
			throw std::runtime_error("World \"" + this->get_identifier() + "\" has no type.");
		}

		if (this->get_star_system() == nullptr) {
			throw std::runtime_error("World \"" + this->get_identifier() + "\" has no star system.");
		}

		this->get_texture_path(); //throws an exception if the texture isn't found

		territory::check();
	}

	void do_day() override;

	virtual std::string get_name() const override;

	virtual void set_county(landed_title *county) override;

	world_type *get_type() const
	{
		return this->type;
	}

	void set_type(world_type *type)
	{
		if (type == this->get_type()) {
			return;
		}

		this->type = type;
		emit type_changed();
	}

	star_system *get_star_system() const
	{
		return this->star_system;
	}

	void set_star_system(star_system *system);

	const std::filesystem::path &get_texture_path() const;

	QString get_texture_path_qstring() const
	{
		return "file:///" + QString::fromStdString(this->get_texture_path().string());
	}

	const QGeoCoordinate &get_astrocoordinate() const
	{
		return this->astrocoordinate;
	}

	void set_astrocoordinate(const QGeoCoordinate &astrocoordinate)
	{
		if (astrocoordinate == this->get_astrocoordinate()) {
			return;
		}

		this->astrocoordinate = astrocoordinate;
		this->calculate_cosmic_map_pos();
	}

	int get_astrodistance() const
	{
		return this->astrodistance;
	}

	void set_astrodistance(const int astrodistance)
	{
		if (astrodistance == this->get_astrodistance()) {
			return;
		}

		this->astrodistance = astrodistance;
		emit astrodistance_changed();
		this->calculate_cosmic_map_pos();
	}

	int get_astrodistance_pc() const
	{
		long long int astrodistance = this->get_astrodistance();
		astrodistance *= 100;
		astrodistance /= world::light_years_per_hundred_parsecs;
		return static_cast<int>(astrodistance);
	}

	void set_astrodistance_pc(const int astrodistance_pc)
	{
		long long int astrodistance = astrodistance_pc;
		astrodistance *= world::light_years_per_hundred_parsecs;
		astrodistance /= 100;
		this->set_astrodistance(static_cast<int>(astrodistance));
	}

	void set_orbit_angle(const double angle);

	void increment_orbit_angle()
	{
		const double increment = 1. * world::min_orbit_distance / this->get_distance_from_orbit_center();
		double angle = this->orbit_angle + increment;
		if (angle >= 360.) {
			angle -= 360.;
		}
		this->set_orbit_angle(angle);
	}

	const QPointF &get_orbit_position() const
	{
		return this->orbit_position;
	}

	void set_orbit_position(const QPointF &position)
	{
		if (position == this->get_orbit_position()) {
			return;
		}

		this->orbit_position = position;
		emit orbit_position_changed();
		this->calculate_cosmic_map_pos();
	}

	world *get_orbit_center() const
	{
		return this->orbit_center;
	}

	void set_orbit_center(world *world)
	{
		if (world == this->get_orbit_center()) {
			return;
		}

		if (this->get_orbit_center() != nullptr) {
			this->get_orbit_center()->remove_satellite(this);
		}

		this->orbit_center = world;

		if (this->get_orbit_center() != nullptr) {
			this->get_orbit_center()->add_satellite(this);
		}

		emit orbit_center_changed();

		this->calculate_cosmic_map_pos();
	}

	bool is_any_orbit_center_of(const world *other_world) const
	{
		if (other_world->get_orbit_center() == nullptr) {
			return false;
		}

		if (other_world->get_orbit_center() == this) {
			return true;
		}

		return this->is_any_orbit_center_of(other_world->get_orbit_center());
	}

	void add_satellite(world *satellite)
	{
		this->satellites.push_back(satellite);
	}

	void remove_satellite(world *satellite);

	double get_distance_from_orbit_center() const
	{
		return this->distance_from_orbit_center;
	}

	void set_distance_from_orbit_center(const double distance)
	{
		if (distance == this->get_distance_from_orbit_center()) {
			return;
		}

		this->distance_from_orbit_center = distance;
		emit distance_from_orbit_center_changed();
		this->calculate_cosmic_map_pos();
	}

	double get_distance_from_orbit_center_au() const
	{
		return this->get_distance_from_orbit_center() / world::million_km_per_au;
	}

	void set_distance_from_orbit_center_au(const double distance_au)
	{
		this->set_distance_from_orbit_center(distance_au * world::million_km_per_au);
	}

	bool is_star() const;

	bool is_planet() const
	{
		return this->get_orbit_center() != nullptr && this->get_orbit_center()->is_star();
	}

	bool is_moon() const
	{
		return this->get_orbit_center() != nullptr && !this->get_orbit_center()->is_star();
	}

	bool has_map() const
	{
		return this->map;
	}

	void set_map(const bool map);

	bool is_map_active() const
	{
		return this->map_active;
	}

	const QSize &get_map_size() const
	{
		return this->map_size;
	}

	int get_surface_area() const
	{
		return this->surface_area;
	}

	int get_area_per_pixel() const
	{
		return this->get_surface_area() / this->get_map_size().width() / this->get_map_size().height();
	}

	int get_radius() const
	{
		return this->radius;
	}

	int get_solar_radius() const
	{
		long long int solar_radius = this->get_radius();
		solar_radius *= 10000;
		solar_radius /= world::solar_radius;
		return static_cast<int>(solar_radius);
	}

	void set_solar_radius(const int solar_radius)
	{
		long long int radius = solar_radius;
		radius *= world::solar_radius;
		radius /= 10000;
		this->radius = static_cast<int>(radius);
	}

	int get_jovian_radius() const
	{
		long long int jovian_radius = this->get_radius();
		jovian_radius *= 10000;
		jovian_radius /= world::jovian_radius;
		return static_cast<int>(jovian_radius);
	}

	void set_jovian_radius(const int jovian_radius)
	{
		long long int radius = jovian_radius;
		radius *= world::jovian_radius;
		radius /= 10000;
		this->radius = static_cast<int>(radius);
	}

	int get_diameter() const
	{
		return this->get_radius() * 2;
	}

	double get_rotation() const
	{
		return this->rotation;
	}

	void set_rotation(const double rotation)
	{
		if (rotation == this->get_rotation()) {
			return;
		}

		this->rotation = rotation;
		emit rotation_changed();
	}

	void rotate()
	{
		//rotate the world on its own axis
		const double increment = 1.;
		double angle = this->get_rotation() + increment;
		if (angle >= 360.) {
			angle -= 360.;
		}
		this->set_rotation(angle);
	}

	const QPointF &get_cosmic_map_pos() const
	{
		return this->cosmic_map_pos;
	}

	void set_cosmic_map_pos(const QPointF &pos)
	{
		if (pos == this->get_cosmic_map_pos()) {
			return;
		}

		this->cosmic_map_pos = pos;
		emit cosmic_map_pos_changed();

		for (world *satellite : this->satellites) {
			satellite->calculate_cosmic_map_pos();
		}
	}

	void calculate_cosmic_map_pos();

	double get_cosmic_size() const
	{
		return this->cosmic_size;
	}

	void set_cosmic_size(const double cosmic_size)
	{
		if (cosmic_size == this->get_cosmic_size()) {
			return;
		}

		this->cosmic_size = cosmic_size;
		this->calculate_cosmic_map_pos();
	}

	double get_cosmic_size_with_satellites() const
	{
		if (!this->satellites.empty()) {
			const world *last_satellite = this->satellites.back();
			return (last_satellite->get_distance_from_orbit_center() + (last_satellite->get_cosmic_size() / 2)) * 2;
		}

		return this->get_cosmic_size();
	}

	void calculate_cosmic_size();

	double get_default_cosmic_size() const
	{
		if (this->is_star()) {
			return world::default_star_size;
		} else if (this->is_planet()) {
			return world::default_planet_size;
		} else if (this->is_moon()) {
			return world::default_moon_size;
		}

		return 0.;
	}

	virtual void add_holding_slot(holding_slot *holding_slot) override;

	bool is_system_capital() const;

	const std::set<province *> &get_provinces() const
	{
		return this->provinces;
	}

	QVariantList get_provinces_qvariant_list() const;

	void add_trade_node(trade_node *node)
	{
		this->trade_nodes.insert(node);
	}

	QVariantList get_trade_nodes_qvariant_list() const;

	const std::set<trade_node *> &get_active_trade_nodes() const
	{
		return this->active_trade_nodes;
	}

	void add_active_trade_node(trade_node *node)
	{
		this->active_trade_nodes.insert(node);
	}

	void remove_active_trade_node(trade_node *node)
	{
		this->active_trade_nodes.erase(node);
	}

	void add_trade_route(trade_route *route)
	{
		this->trade_routes.insert(route);
	}

	QVariantList get_trade_routes_qvariant_list() const;

	std::filesystem::path get_cache_path() const
	{
		return database::get_cache_path() / this->get_identifier();
	}

	QString get_cache_path_qstring() const
	{
		return "file:///" + QString::fromStdString(this->get_cache_path().string());
	}

	bool is_pos_valid(const QPoint &pos) const
	{
		return pos.x() > 0 && pos.y() > 0 && pos.x() < this->get_map_size().width() && pos.y() < this->get_map_size().height();
	}

	QPoint get_pixel_pos(const int index) const;
	QPoint get_coordinate_pos(const QGeoCoordinate &coordinate) const;

	double get_lon_per_pixel() const
	{
		return 360.0 / static_cast<double>(this->get_map_size().width());
	}

	double get_lat_per_pixel() const
	{
		return 180.0 / static_cast<double>(this->get_map_size().height());
	}

	QRect get_georectangle_rect(const QGeoRectangle &georectangle) const
	{
		const QPoint top_left = this->get_coordinate_pos(georectangle.topLeft());
		const QPoint bottom_right = this->get_coordinate_pos(georectangle.bottomRight());

		return QRect(top_left, bottom_right);
	}

	QGeoCoordinate get_pixel_pos_coordinate(const QPoint &pos) const;
	terrain_type *get_coordinate_terrain(const QGeoCoordinate &coordinate) const;
	province *get_pos_province(const QPoint &pos) const;
	province *get_coordinate_province(const QGeoCoordinate &coordinate) const;

	const pathfinder *get_pathfinder() const
	{
		return this->pathfinder.get();
	}

	template <typename T>
	std::vector<gsml_data> parse_data_type_map_database() const
	{
		std::vector<gsml_data> gsml_map_data_to_process;

		for (const std::filesystem::path &path : database::get()->get_map_paths()) {
			std::filesystem::path map_path = path / this->get_identifier() / T::database_folder;

			if (!std::filesystem::exists(map_path)) {
				continue;
			}

			database::parse_folder(map_path, gsml_map_data_to_process);
		}

		return gsml_map_data_to_process;
	}

	std::vector<QVariantList> parse_geojson_folder(const std::string_view &folder) const;

	template <typename T>
	void process_data_type_map_geojson_database()
	{
		if (std::string_view(T::database_folder).empty()) {
			return;
		}

		const std::vector<QVariantList> geojson_data_list = this->parse_geojson_folder(T::database_folder);

		for (const QVariantList &geojson_data : geojson_data_list) {
			const QVariantMap feature_collection = geojson_data.front().toMap();
			const QVariantList feature_collection_data = feature_collection.value("data").toList();

			for (const QVariant &feature_variant : feature_collection_data) {
				const QVariantMap feature = feature_variant.toMap();
				T::process_geojson_feature(feature);
			}
		}
	}

	void process_province_map_database();
	void process_terrain_map_database();
	void process_terrain_gsml_data(const terrain_type *terrain, const gsml_data &data);
	void process_terrain_gsml_scope(const terrain_type *terrain, const gsml_data &scope);

	void load_province_map();
	void load_terrain_map();

	void write_geodata_to_image();
	void write_terrain_geodata_to_image(QImage &terrain_image);
	void write_province_geodata_to_image(QImage &province_image, QImage &terrain_image);
	void write_terrain_geoshape_to_image(const terrain_type *terrain, QImage &image, const QGeoShape &geoshape);
	void write_terrain_geopath_endpoints_to_image(QImage &image);

	QString get_loading_message_name() const
	{
		if (world::get_map_worlds().size() > 1) {
			return this->get_name_qstring();
		} else {
			return "World";
		}
	}

	Q_INVOKABLE QPoint coordinate_to_point(const QGeoCoordinate &coordinate) const
	{
		return this->get_coordinate_pos(coordinate);
	}

	void amalgamate();

private:
	void add_province(province *province);

signals:
	void type_changed();
	void star_system_changed();
	void astrodistance_changed();
	void orbit_center_changed();
	void distance_from_orbit_center_changed();
	void orbit_position_changed();
	void rotation_changed();
	void cosmic_map_pos_changed();

private:
	world_type *type = nullptr;
	star_system *star_system = nullptr;
	QGeoCoordinate astrocoordinate;
	int astrodistance = 0;
	double orbit_angle = 0; //in degrees
	QPointF orbit_position;
	world *orbit_center = nullptr; //if none is given, then the center of the star system is assumed
	double rotation = 0.; //the rotation of the world on its own axis
	QPointF cosmic_map_pos;
	double distance_from_orbit_center = 0; //in millions of kilometers
	bool map = false; //whether the world has a map
	bool map_active = false; //whether the world's map is active
	int surface_area = 0; //the world's surface area, in square kilometers
	int radius = 0; //the world's radius, in kilometers
	std::vector<world *> satellites;
	std::set<province *> provinces;
	std::set<trade_node *> trade_nodes; //the trade nodes in the world
	std::set<trade_node *> active_trade_nodes; //the active trade nodes in the world
	std::set<trade_route *> trade_routes; //the trade routes in the world
	QSize map_size = QSize(0, 0); //the size of the world's map, in pixels
	double cosmic_size = 0; //the size of the world for the cosmic map
	QImage terrain_image;
	QImage province_image;
	std::map<const terrain_type *, std::vector<QGeoPolygon>> terrain_geopolygons;
	std::map<const terrain_type *, std::vector<QGeoPath>> terrain_geopaths;
	std::unique_ptr<pathfinder> pathfinder;
};

}
