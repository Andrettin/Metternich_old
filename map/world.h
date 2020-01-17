#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QtLocation/private/qgeojson_p.h>

#include <filesystem>
#include <set>

namespace metternich {

class pathfinder;
class province;
class terrain_type;
class trade_node;
class trade_route;

class world : public data_entry, public data_type<world>
{
	Q_OBJECT

	Q_PROPERTY(int surface_area MEMBER surface_area READ get_surface_area)
	Q_PROPERTY(QVariantList provinces READ get_provinces_qvariant_list CONSTANT)
	Q_PROPERTY(QVariantList trade_nodes READ get_trade_nodes_qvariant_list CONSTANT)
	Q_PROPERTY(QVariantList trade_routes READ get_trade_routes_qvariant_list CONSTANT)
	Q_PROPERTY(QString cache_path READ get_cache_path_qstring CONSTANT)

public:
	static constexpr const char *class_identifier = "world";
	static constexpr const char *database_folder = "worlds";

public:
	world(const std::string &identifier);
	virtual ~world() override;

	virtual void initialize() override;

	int get_surface_area() const
	{
		return this->surface_area;
	}

	int get_area_per_pixel() const
	{
		return this->get_surface_area() / this->pixel_size.width() / this->pixel_size.height();
	}

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
		return pos.x() > 0 && pos.y() > 0 && pos.x() < this->pixel_size.width() && pos.y() < this->pixel_size.height();
	}

	QPoint get_pixel_pos(const int index) const;
	QPoint get_coordinate_pos(const QGeoCoordinate &coordinate) const;

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
	inline std::vector<gsml_data> parse_data_type_map_database() const
	{
		std::vector<gsml_data> gsml_map_data_to_process;

		for (const std::filesystem::path &path : database::get()->get_map_paths()) {
			std::filesystem::path map_path = path / this->get_identifier() / T::database_folder;

			if (!std::filesystem::exists(map_path)) {
				continue;
			}

			std::filesystem::recursive_directory_iterator dir_iterator(map_path);

			for (const std::filesystem::directory_entry &dir_entry : dir_iterator) {
				if (!dir_entry.is_regular_file() || dir_entry.path().extension() != ".txt") {
					continue;
				}

				if (T::try_get(dir_entry.path().stem().string()) == nullptr) {
					throw std::runtime_error(dir_entry.path().stem().string() + " is not a valid " + T::class_identifier + " instance identifier.");
				}

				gsml_parser parser(dir_entry.path());
				gsml_map_data_to_process.push_back(parser.parse());
			}
		}

		return gsml_map_data_to_process;
	}

	template <typename T>
	inline std::vector<QVariantList> parse_data_type_map_geojson_database() const
	{
		std::vector<QVariantList> geojson_data_list;

		for (const std::filesystem::path &path : database::get()->get_map_paths()) {
			std::filesystem::path map_path = path / this->get_identifier() / T::database_folder;

			if (!std::filesystem::exists(map_path)) {
				continue;
			}

			std::filesystem::recursive_directory_iterator dir_iterator(map_path);

			for (const std::filesystem::directory_entry &dir_entry : dir_iterator) {
				if (!dir_entry.is_regular_file() || dir_entry.path().extension() != ".geojson") {
					continue;
				}

				std::ifstream ifstream(dir_entry);

				if (!ifstream) {
					throw std::runtime_error("Failed to open file: " + dir_entry.path().string());
				}

				const std::string raw_geojson_data(std::istreambuf_iterator<char>{ifstream}, std::istreambuf_iterator<char>{});
				const QByteArray raw_geojson_byte_array = QByteArray::fromStdString(raw_geojson_data);

				QJsonParseError json_error;
				const QJsonDocument json = QJsonDocument::fromJson(raw_geojson_byte_array, &json_error);

				if (json.isNull()) {
					throw std::runtime_error("JSON parsing failed: " + json_error.errorString().toStdString() + ".");
				}

				QVariantList geojson_data = QGeoJson::importGeoJson(json);
				geojson_data_list.push_back(std::move(geojson_data));
			}
		}

		return geojson_data_list;
	}

	void process_province_map_database();
	void process_holding_slot_map_database();

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
		if (world::get_all().size() > 1) {
			return this->get_name_qstring();
		} else {
			return "World";
		}
	}


	Q_INVOKABLE QPoint coordinate_to_point(const QGeoCoordinate &coordinate) const
	{
		return this->get_coordinate_pos(coordinate);
	}

private:
	void add_province(province *province);

private:
	int surface_area = 0; //the world's surface area, in square kilometers
	std::set<province *> provinces;
	std::set<trade_node *> trade_nodes; //the trade nodes in the world
	std::set<trade_node *> active_trade_nodes; //the active trade nodes in the world
	std::set<trade_route *> trade_routes; //the trade routes in the world
	QSize pixel_size = QSize(0, 0); //the size of the world, in pixels
	QImage terrain_image;
	QImage province_image;
	std::map<const terrain_type *, std::vector<QGeoPolygon>> terrain_geopolygons;
	std::map<const terrain_type *, std::vector<QGeoPath>> terrain_geopaths;
	std::unique_ptr<pathfinder> pathfinder;
};

}
