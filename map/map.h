#pragma once

#include "database/gsml_data.h"
#include "singleton.h"

#include <QGeoCoordinate>
#include <QPoint>
#include <QSize>
#include <QVariantList>

#include <utility>

namespace metternich {

class province;
class terrain_type;
class world;
enum class map_mode : int;

class map : public singleton<map>
{
public:
	static std::vector<QVariantList> parse_geojson_folder(const std::filesystem::path &path);

private:
	static std::vector<QVariantList> parse_world_geojson_database();
	static void process_world_geojson_database();

	static constexpr int geocoordinate_precision = 17;
	static constexpr int cosmic_map_boundary_offset = 128; //the offset from the celestial body at the cosmic map's boundary for determining its bounding rect

public:
	map();
	void load();

	world *get_current_world() const
	{
		return this->current_world;
	}

	void set_current_world(world *world);

	map_mode get_mode() const
	{
		return this->mode;
	}

	void set_mode(const map_mode mode);

	const QRectF &get_cosmic_map_bounding_rect() const
	{
		return this->cosmic_map_bounding_rect;
	}

	void calculate_cosmic_map_bounding_rect();

private:
	void load_geojson_files();
	void process_geojson_data(const QVariantMap &geojson_data);
	void process_geojson_features(const QVariantList &features);
	void process_geojson_polygon(const std::string &feature_name, const QVariantList &coordinate_group);
	void process_geojson_line(const std::string &feature_name, const QVariantList &coordinates);
	void process_geojson_coordinates(const QVariantList &coordinates, gsml_data &coordinate_list_data);
	void save_geojson_data_to_gsml();
	bool check_cache();
	void save_cache();

private:
	std::map<std::string, std::vector<gsml_data>> geojson_polygon_data; //GeoJSON geopolygon coordinates, mapped to the name of the corresponding feature
	std::map<std::string, std::vector<gsml_data>> geojson_path_data; //GeoJSON geopath coordinates, mapped to the name of the corresponding feature
	std::string checksum;
	world *current_world = nullptr;
	map_mode mode;
	QRectF cosmic_map_bounding_rect;
};

}
