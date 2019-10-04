#pragma once

#include "singleton.h"

#include <QPoint>
#include <QSize>
#include <QVariantList>

namespace metternich {

class map : public singleton<map>
{
private:
	static constexpr int geojson_coordinate_precision = 15;

public:
	void load();
	QPoint get_pixel_position(const int index);

private:
	void load_geojson_files();
	void process_geojson_data(const QVariantMap &geojson_data);
	void process_geojson_features(const QVariantList &features);
	void process_geojson_polygon_coordinates(const std::string &feature_name, const QVariantList &coordinates);
	void save_geojson_data_to_gsml();
	void load_provinces();
	void load_terrain();

private:
	QSize size = QSize(0, 0);
	std::map<std::string, std::vector<std::vector<std::pair<double, double>>>> geojson_polygon_coordinates; //GeoJSON polygon coordinates, mapped to the name of the corresponding feature
};

}
