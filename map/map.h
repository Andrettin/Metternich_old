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
	static std::vector<QVariantList> parse_geojson_folder(const std::filesystem::path &path)
	{
		std::vector<QVariantList> geojson_data_list;

		std::filesystem::recursive_directory_iterator dir_iterator(path);

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

		return geojson_data_list;
	}

private:
	static constexpr int geocoordinate_precision = 17;

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
};

}
