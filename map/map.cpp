#include "map/map.h"

#include "engine_interface.h"
#include "map/province.h"
#include "map/terrain_type.h"
#include "util/filesystem_util.h"
#include "util/point_util.h"

#include <QCryptographicHash>
#include <QImage>
#include <QJsonDocument>
#include <QRect>

#include <sstream>

namespace metternich {

void map::load()
{
	this->load_geojson_files();
	this->load_provinces();
	this->load_terrain();

	//load map data for terrain types and provinces
	province::process_map_database();

	if (this->check_cache()) {
		EngineInterface::get()->set_loading_message("Loading Map Cache...");
		terrain_type::process_cache();
		province::process_cache();
	} else {
		terrain_type::process_map_database(); //we only need to load the terrain map database to calculate the province terrain

		int processed_provinces = 0;
		for (province *province : province::get_all()) {
			const int progress_percent = processed_provinces * 100 / static_cast<int>(province::get_all().size());
			EngineInterface::get()->set_loading_message("Calculating Province Terrain and Border Provinces... (" + QString::number(progress_percent) + "%)");

			if (province->get_terrain() == nullptr) {
				province->calculate_terrain();
			}

			province->calculate_border_provinces();
			processed_provinces++;
		}

		this->save_cache();
	}
}

QPoint map::get_pixel_position(const int index) const
{
	return util::index_to_point(index, this->size);
}

terrain_type *map::get_coordinate_terrain(const QGeoCoordinate &coordinate) const
{
	for (terrain_type *terrain_type : terrain_type::get_all()) {
		if (terrain_type->contains_coordinate(coordinate)) {
			return terrain_type;
		}
	}

	return nullptr;
}

province *map::get_coordinate_province(const QGeoCoordinate &coordinate) const
{
	for (province *province : province::get_all()) {
		if (province->contains_coordinate(coordinate)) {
			return province;
		}
	}

	return nullptr;
}

/**
**	@brief	Load GeoJSON files and save their data in GSML files
*/
void map::load_geojson_files()
{
	std::filesystem::path province_data_path("./map/provinces");

	if (!std::filesystem::exists(province_data_path)) {
		return;
	}

	std::filesystem::recursive_directory_iterator dir_iterator(province_data_path);

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

		this->process_geojson_data(json.toVariant().toMap());
	}

	this->save_geojson_data_to_gsml();
}

/**
**	@brief	Process GeoJSON data
**
**	@param	geojson_data	The GeoJSON data
*/
void map::process_geojson_data(const QVariantMap &geojson_data)
{
	const QString geojson_type = geojson_data.value("type").toString();

	if (geojson_type == "FeatureCollection") {
		this->process_geojson_features(geojson_data.value("features").toList());
	} else {
		throw std::runtime_error("Invalid GeoJSON entry type: " + geojson_type.toStdString());
	}
}

/**
**	@brief	Process a list of GeoJSON features
**
**	@param	features	The GeoJSON features
*/
void map::process_geojson_features(const QVariantList &features)
{
	for (const QVariant &feature_variant : features) {
		const QVariantMap feature = feature_variant.toMap();
		const QVariantMap feature_properties = feature.value("properties").toMap();
		const std::string feature_name = feature_properties.value("name").toString().toStdString();
		const QVariantMap geometry = feature.value("geometry").toMap();
		const std::string geometry_type = geometry.value("type").toString().toStdString();

		if (geometry_type == "MultiPolygon") {
			const QVariantList multi_polygon_coordinates = geometry.value("coordinates").toList();

			for (const QVariant &polygon_coordinates_variant : multi_polygon_coordinates) {
				const QVariantList polygon_coordinates = polygon_coordinates_variant.toList();
				this->process_geojson_polygon(feature_name, polygon_coordinates);
			}
		} else {
			throw std::runtime_error("Invalid GeoJSON feature type: " + geometry_type);
		}
	}
}

/**
**	@brief	Process the coordinates for a GeoJSON polygon
**
**	@param	feature_name		The name of the feature
**	@param	coordinate_group	The coordinates, containing both the polygon itself and holes in it (if any)
*/
void map::process_geojson_polygon(const std::string &feature_name, const QVariantList &coordinate_group)
{
	gsml_data geopolygon_data;

	const QVariantList coordinates = coordinate_group.front().toList();
	gsml_data coordinate_data("coordinates");
	this->process_geojson_coordinates(coordinates, coordinate_data);
	geopolygon_data.add_child(std::move(coordinate_data));

	//process hole coordinates
	if (coordinate_group.size() > 1) {
		gsml_data hole_data("hole_coordinates");
		for (int i = 1; i < coordinate_group.size(); ++i) {
			const QVariantList hole_coordinates = coordinate_group[i].toList();
			gsml_data hole_coordinate_data;
			this->process_geojson_coordinates(hole_coordinates, hole_coordinate_data);
			hole_data.add_child(std::move(hole_coordinate_data));
		}
		geopolygon_data.add_child(std::move(hole_data));
	}

	this->geojson_polygon_data[feature_name].push_back(std::move(geopolygon_data));
}

/**
**	@brief	Process GeoJSON coordinates
**
**	@param	coordinates				The coordinates
**	@param	coordinate_list_data	The coordinate list GSML data
*/
void map::process_geojson_coordinates(const QVariantList &coordinates, gsml_data &coordinate_list_data)
{
	for (const QVariant &coordinate_variant : coordinates) {
		const QVariantList coordinate = coordinate_variant.toList();
		const double longitude = coordinate[0].toDouble();
		const double latitude = coordinate[1].toDouble();

		gsml_data coordinate_data;

		std::ostringstream lon_string_stream;
		lon_string_stream << std::setprecision(map::geojson_coordinate_precision) << longitude;
		coordinate_data.add_value(lon_string_stream.str());

		std::ostringstream lat_string_stream;
		lat_string_stream << std::setprecision(map::geojson_coordinate_precision) << latitude;
		coordinate_data.add_value(lat_string_stream.str());

		coordinate_list_data.add_child(std::move(coordinate_data));
	}
}

/**
**	@brief	Save GeoJSON data as GSML files
*/
void map::save_geojson_data_to_gsml()
{
	for (auto &kv_pair : this->geojson_polygon_data) {
		const std::string &feature_name = kv_pair.first;

		gsml_data data(feature_name);
		gsml_data geopolygons("geopolygons");

		for (gsml_data &geopolygon_data : kv_pair.second) {
			geopolygons.add_child(std::move(geopolygon_data));
		}

		data.add_child(std::move(geopolygons));
		data.print_to_dir("./map/provinces/");
	}

	this->geojson_polygon_data.clear();
}

void map::load_provinces()
{
	EngineInterface::get()->set_loading_message("Loading Provinces... (0%)");

	QImage province_image("./map/provinces.png");
	QImage terrain_image("./map/terrain.png"); //used to calculate each province's terrain
	this->size = province_image.size(); //set the map's size to that of the province map
	const int pixel_count = province_image.width() * province_image.height();

	std::map<province *, std::vector<int>> province_pixel_indexes;
	std::map<province *, std::vector<int>> province_border_pixel_indexes;
	std::map<province *, std::map<terrain_type *, int>> province_terrain_counts;

	const QRgb *rgb_data = reinterpret_cast<const QRgb *>(province_image.constBits());
	const QRgb *terrain_rgb_data = reinterpret_cast<const QRgb *>(terrain_image.constBits());

	province *previous_pixel_province = nullptr; //used to see which provinces border which horizontally
	for (int i = 0; i < pixel_count; ++i) {
		if ((i % province_image.width()) == 0) {
			//new line, set the previous pixel province to null
			previous_pixel_province = nullptr;

			//update the progress in the loading message
			const long long int progress_percent = static_cast<long long int>(i) * 100 / pixel_count;
			EngineInterface::get()->set_loading_message("Loading Provinces... (" + QString::number(progress_percent) + "%)");
		}

		const QRgb &pixel_rgb = rgb_data[i];

		province *pixel_province = province::get_by_rgb(pixel_rgb, false);
		if (pixel_province != nullptr) {
			province_pixel_indexes[pixel_province].push_back(i);

			if (previous_pixel_province != pixel_province && previous_pixel_province != nullptr) {
				province_border_pixel_indexes[pixel_province].push_back(i);
				province_border_pixel_indexes[previous_pixel_province].push_back(i - 1);
				pixel_province->add_border_province(previous_pixel_province);
				previous_pixel_province->add_border_province(pixel_province);
			}

			if (i > province_image.width()) { //second line or below
				//the pixel just above this one
				const QRgb &previous_vertical_pixel_rgb = rgb_data[i - province_image.width()];
				province *previous_vertical_pixel_province = province::get_by_rgb(previous_vertical_pixel_rgb, false);
				if (previous_vertical_pixel_province != pixel_province && previous_vertical_pixel_province != nullptr) {
					province_border_pixel_indexes[pixel_province].push_back(i);
					province_border_pixel_indexes[previous_vertical_pixel_province].push_back(i - province_image.width());
					pixel_province->add_border_province(previous_vertical_pixel_province);
					previous_vertical_pixel_province->add_border_province(pixel_province);
				}
			}

			const QRgb &terrain_pixel_rgb = terrain_rgb_data[i];
			terrain_type *pixel_terrain = terrain_type::get_by_rgb(terrain_pixel_rgb);
			std::map<terrain_type *, int> &province_terrain_count = province_terrain_counts[pixel_province];
			if (province_terrain_count.find(pixel_terrain) == province_terrain_count.end()) {
				province_terrain_count[pixel_terrain] = 0;
			}
			province_terrain_count[pixel_terrain]++;
		}

		previous_pixel_province = pixel_province;
	}

	for (const auto &province_terrain_count : province_terrain_counts) {
		province *province = province_terrain_count.first;
		terrain_type *best_terrain = nullptr;
		int best_terrain_count = 0;
		for (const auto &kv_pair : province_terrain_count.second) {
			terrain_type *terrain = kv_pair.first;
			const int count = kv_pair.second;
			if (count > best_terrain_count) {
				best_terrain = terrain;
				best_terrain_count = count;
			}
		}
		province->set_terrain(best_terrain);
	}

	for (const auto &kv_pair : province_pixel_indexes) {
		province *province = kv_pair.first;
		province->create_image(kv_pair.second);
	}

	for (const auto &kv_pair : province_border_pixel_indexes) {
		province *province = kv_pair.first;
		province->set_border_pixels(kv_pair.second);
	}
}

void map::load_terrain()
{
	EngineInterface::get()->set_loading_message("Loading Terrain...");
	QImage terrain_image("./map/terrain.png");
}

/**
**	@brief	Check to see if the cache is valid
**
**	@return	True if the cache is valid, or false otherwise
*/
bool map::check_cache()
{
	QCryptographicHash hash(QCryptographicHash::Md5);
	util::add_files_to_checksum(hash, "./map/provinces");
	util::add_files_to_checksum(hash, "./map/terrain_types");

	this->checksum = hash.result().toHex().toStdString(); //save the checksum

	const std::filesystem::path cache_path("./cache");

	if (!std::filesystem::exists(cache_path)) {
		return false;
	}

	const std::filesystem::path stored_checksum_path(cache_path / "checksum.txt");

	if (!std::filesystem::exists(stored_checksum_path)) {
		return false;
	}

	gsml_parser parser(stored_checksum_path);
	gsml_data stored_checksum_data = parser.parse();
	std::string stored_checksum = stored_checksum_data.get_property_value("map");

	if (stored_checksum != this->checksum) {
		return false;
	}

	return true;
}

/**
**	@brief	Save the map cache
*/
void map::save_cache()
{
	const std::filesystem::path cache_path("./cache");

	if (!std::filesystem::exists(cache_path)) {
		std::filesystem::create_directory(cache_path);
	}

	const std::filesystem::path stored_checksum_path(cache_path / "checksum.txt");

	std::ofstream ofstream(stored_checksum_path);
	gsml_data stored_checksum_data;
	stored_checksum_data.add_property("map", this->checksum);
	stored_checksum_data.print_components(ofstream);
	ofstream.close();

	province::save_cache();
}

}
