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

struct geocoordinate_int
{
	geocoordinate_int(const int lat, const int lon) : latitude(lat), longitude(lon)
	{
	}

	geocoordinate_int(const double lat, const double lon) : latitude(static_cast<int>(lat)), longitude(static_cast<int>(lon))
	{
	}

	geocoordinate_int(const QGeoCoordinate &coordinate) : geocoordinate_int(coordinate.latitude(), coordinate.longitude())
	{
	}

	int latitude = 0;
	int longitude = 0;
};

class map : public singleton<map>
{
private:
	static constexpr int geojson_coordinate_precision = 17;

public:
	void load();
	QPoint get_pixel_position(const int index) const;
	terrain_type *get_coordinate_terrain(const QGeoCoordinate &coordinate) const;
	province *get_coordinate_province(const QGeoCoordinate &coordinate) const;

private:
	void load_geojson_files();
	void process_geojson_data(const QVariantMap &geojson_data);
	void process_geojson_features(const QVariantList &features);
	void process_geojson_polygon(const std::string &feature_name, const QVariantList &coordinate_group);
	void process_geojson_line(const std::string &feature_name, const QVariantList &coordinates);
	void process_geojson_coordinates(const QVariantList &coordinates, gsml_data &coordinate_list_data);
	void save_geojson_data_to_gsml();
	void load_provinces();
	void load_terrain();
	bool check_cache();
	void save_cache();
	void update_province_image_from_geodata();

private:
	QSize size = QSize(0, 0);
	std::map<std::string, std::vector<gsml_data>> geojson_polygon_data; //GeoJSON geopolygon coordinates, mapped to the name of the corresponding feature
	std::map<std::string, std::vector<gsml_data>> geojson_path_data; //GeoJSON geopath coordinates, mapped to the name of the corresponding feature
	std::string checksum;
	std::map<int, std::map<int, std::vector<std::pair<const QGeoPolygon *, terrain_type *>>>> terrain_geopolygons_by_int_coordinate; //list of terrain type geopolygons located in a given integer geocoordinate square
	std::map<int, std::map<int, std::vector<province *>>> provinces_by_int_coordinate; //list of provinces located in a given geocoordinate square
};

}
