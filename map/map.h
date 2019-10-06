#pragma once

#include "database/gsml_data.h"
#include "singleton.h"

#include <QPoint>
#include <QSize>
#include <QVariantList>

#include <utility>

namespace metternich {

class map : public singleton<map>
{
private:
	static constexpr int geojson_coordinate_precision = 17;

public:
	void load();
	QPoint get_pixel_position(const int index);

private:
	void load_geojson_files();
	void process_geojson_data(const QVariantMap &geojson_data);
	void process_geojson_features(const QVariantList &features);
	void process_geojson_polygon(const std::string &feature_name, const QVariantList &coordinate_group);
	void process_geojson_coordinates(const QVariantList &coordinates, gsml_data &coordinate_list_data);
	void save_geojson_data_to_gsml();
	void load_provinces();
	void load_terrain();

private:
	QSize size = QSize(0, 0);
	std::map<std::string, std::vector<gsml_data>> geojson_polygon_data; //GeoJSON geopolygons coordinates, mapped to the name of the corresponding feature
};

}
