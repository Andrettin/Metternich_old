#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <filesystem>
#include <set>

namespace metternich {

class province;
class terrain_type;
class trade_route;

class world : public data_entry, public data_type<world>
{
	Q_OBJECT

	Q_PROPERTY(QVariantList provinces READ get_provinces_qvariant_list CONSTANT)
	Q_PROPERTY(QVariantList trade_routes READ get_trade_routes_qvariant_list CONSTANT)
	Q_PROPERTY(QString cache_path READ get_cache_path_qstring CONSTANT)

public:
	static constexpr const char *class_identifier = "world";
	static constexpr const char *database_folder = "worlds";

public:
	world(const std::string &identifier) : data_entry(identifier) {}

	virtual void initialize() override
	{
		if (this->terrain_image.size() != this->province_image.size()) {
			throw std::runtime_error("The terrain and province images of world \"" + this->get_identifier() + "\" have different sizes.");
		}

		//clear the terrain and province images, as there is no need to keep them in memory
		this->terrain_image = QImage();
		this->province_image = QImage();

		data_entry_base::initialize();
	}

	const std::set<province *> &get_provinces() const
	{
		return this->provinces;
	}

	void add_trade_route(trade_route *route)
	{
		this->trade_routes.insert(route);
	}

	const std::set<province *> &get_geopath_provinces()
	{
		return this->geopath_provinces;
	}

	QVariantList get_provinces_qvariant_list() const;
	QVariantList get_trade_routes_qvariant_list() const;

	std::filesystem::path get_cache_path() const
	{
		return database::get_cache_path() / this->get_identifier();
	}

	QString get_cache_path_qstring() const
	{
		return "file:///" + QString::fromStdString(this->get_cache_path().string());
	}

	QPoint get_pixel_pos(const int index) const;
	QPoint get_coordinate_pos(const QGeoCoordinate &coordinate) const;
	QGeoCoordinate get_pixel_pos_coordinate(const QPoint &pos) const;
	terrain_type *get_coordinate_terrain(const QGeoCoordinate &coordinate) const;
	province *get_coordinate_province(const QGeoCoordinate &coordinate) const;

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
	std::set<province *> provinces;
	std::set<trade_route *> trade_routes; //the trade routes which exist in the world
	std::set<province *> geopath_provinces;
	QSize pixel_size = QSize(0, 0); //the size of the world, in pixels
	QImage terrain_image;
	QImage province_image;
	std::map<const terrain_type *, std::vector<QGeoPolygon>> terrain_geopolygons;
	std::map<const terrain_type *, std::vector<QGeoPath>> terrain_geopaths;
};

}
