#include "map/map.h"

#include "engine_interface.h"
#include "map/province.h"
#include "map/terrain_type.h"
#include "util/filesystem_util.h"
#include "util/image_util.h"
#include "util/location_util.h"
#include "util/point_util.h"

#include <QCryptographicHash>
#include <QGeoRectangle>
#include <QImage>
#include <QJsonDocument>
#include <QRect>

#include <sstream>

namespace metternich {

void map::load()
{
	const bool cache_valid = this->check_cache();

	if (cache_valid) {
		EngineInterface::get()->set_loading_message("Loading Map Cache...");
		terrain_type::process_cache();
		province::process_cache();
	} else {
		EngineInterface::get()->set_loading_message("Building Map Cache...");

		//clear cache
		std::filesystem::path cache_path = database::get_cache_path();
		std::filesystem::remove_all(cache_path);
		std::filesystem::create_directories(cache_path);

		this->load_geojson_files();

		//load map data for terrain types and provinces
		province::process_map_database();
		terrain_type::process_map_database();

		this->write_geodata_to_image();
	}

	this->load_terrain();
	this->load_provinces();

	if (!cache_valid) {
		this->save_cache();
	}

	//clear the terrain and province images, as there is no need to keep them in memory
	this->terrain_image = QImage();
	this->province_image = QImage();
}

QPoint map::get_pixel_pos(const int index) const
{
	return util::index_to_point(index, this->size);
}

/**
**	@brief	Convert a coordinate to a pixel position on the map
**
**	@param	coordinate	The geocoordinate
**
**	@return The pixel position corresponding to the coordinate
*/
QPoint map::get_coordinate_pos(const QGeoCoordinate &coordinate) const
{
	const double lon_per_pixel = 360.0 / static_cast<double>(this->size.width());
	const double lat_per_pixel = 180.0 / static_cast<double>(this->size.height());
	return util::coordinate_to_point(coordinate, lon_per_pixel, lat_per_pixel);
}

terrain_type *map::get_coordinate_terrain(const QGeoCoordinate &coordinate) const
{
	if (this->terrain_image.isNull()) {
		throw std::runtime_error("Cannot get coordinate terrain after clearing the terrain image from memory.");
	}

	QPoint pos = this->get_coordinate_pos(coordinate);
	QRgb rgb = this->terrain_image.pixel(pos);
	return terrain_type::get_by_rgb(rgb);
}

province *map::get_coordinate_province(const QGeoCoordinate &coordinate) const
{
	if (this->province_image.isNull()) {
		throw std::runtime_error("Cannot get coordinate province after clearing the province image from memory.");
	}

	QPoint pos = this->get_coordinate_pos(coordinate);
	QRgb rgb = this->province_image.pixel(pos);
	return province::get_by_rgb(rgb);
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
		} else if (geometry_type == "MultiLineString") {
			const QVariantList multi_line_coordinates = geometry.value("coordinates").toList();

			for (const QVariant &line_coordinates_variant : multi_line_coordinates) {
				const QVariantList line_coordinates = line_coordinates_variant.toList();
				this->process_geojson_line(feature_name, line_coordinates);
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
**	@brief	Process the coordinates for a GeoJSON line
**
**	@param	feature_name	The name of the feature
**	@param	coordinates		The coordinates for the line
*/
void map::process_geojson_line(const std::string &feature_name, const QVariantList &coordinates)
{
	gsml_data geopath_data;

	gsml_data coordinate_data("coordinates");
	this->process_geojson_coordinates(coordinates, coordinate_data);
	geopath_data.add_child(std::move(coordinate_data));

	this->geojson_path_data[feature_name].push_back(std::move(geopath_data));
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

	for (auto &kv_pair : this->geojson_path_data) {
		const std::string &feature_name = kv_pair.first;

		gsml_data data(feature_name);
		gsml_data geopaths("geopaths");

		for (gsml_data &geopath_data : kv_pair.second) {
			geopaths.add_child(std::move(geopath_data));
		}

		data.add_child(std::move(geopaths));
		data.print_to_dir("./map/provinces/");
	}

	this->geojson_path_data.clear();
}

void map::load_provinces()
{
	EngineInterface::get()->set_loading_message("Loading Provinces... (0%)");

	this->province_image = QImage(QString::fromStdString((database::get_cache_path() / "provinces.png").string()));

	this->size = this->province_image.size(); //set the map's size to that of the province map
	const int pixel_count = this->province_image.width() * this->province_image.height();

	std::map<province *, std::vector<int>> province_pixel_indexes;
	std::map<province *, std::vector<int>> province_border_pixel_indexes;
	std::map<province *, std::map<terrain_type *, int>> province_terrain_counts;

	const QRgb *rgb_data = reinterpret_cast<const QRgb *>(this->province_image.constBits());
	const QRgb *terrain_rgb_data = reinterpret_cast<const QRgb *>(this->terrain_image.constBits());

	province *previous_pixel_province = nullptr; //used to see which provinces border which horizontally
	for (int i = 0; i < pixel_count; ++i) {
		const bool line_start = ((i % this->province_image.width()) == 0);
		if (line_start) {
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

			if (i > this->province_image.width()) { //second line or below
				//the pixel just above this one
				const int adjacent_pixel_index = i - this->province_image.width();
				const QRgb &previous_vertical_pixel_rgb = rgb_data[adjacent_pixel_index];
				province *previous_vertical_pixel_province = province::get_by_rgb(previous_vertical_pixel_rgb, false);
				if (previous_vertical_pixel_province != pixel_province && previous_vertical_pixel_province != nullptr) {
					province_border_pixel_indexes[pixel_province].push_back(i);
						province_border_pixel_indexes[previous_vertical_pixel_province].push_back(adjacent_pixel_index);
					pixel_province->add_border_province(previous_vertical_pixel_province);
					previous_vertical_pixel_province->add_border_province(pixel_province);
				}
			}

			const QRgb &terrain_pixel_rgb = terrain_rgb_data[i];
			terrain_type *pixel_terrain = terrain_type::get_by_rgb(terrain_pixel_rgb);
			std::map<terrain_type *, int> &province_terrain_count = province_terrain_counts[pixel_province];
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
	this->terrain_image = QImage(QString::fromStdString((database::get_cache_path() / "terrain.png").string()));
}

/**
**	@brief	Check to see if the cache is valid
**
**	@return	True if the cache is valid, or false otherwise
*/
bool map::check_cache()
{
	QCryptographicHash hash(QCryptographicHash::Md5);
	util::add_files_to_checksum(hash, "./map");

	this->checksum = hash.result().toHex().toStdString(); //save the checksum

	const std::filesystem::path cache_path = database::get_cache_path();

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
	const std::filesystem::path cache_path = database::get_cache_path();

	if (!std::filesystem::exists(cache_path)) {
		std::filesystem::create_directories(cache_path);
	}

	const std::filesystem::path stored_checksum_path(cache_path / "checksum.txt");

	std::ofstream ofstream(stored_checksum_path);
	gsml_data stored_checksum_data;
	stored_checksum_data.add_property("map", this->checksum);
	stored_checksum_data.print_components(ofstream);
	ofstream.close();

	province::save_cache();
}

/**
**	@brief	Write geodata to image files
*/
void map::write_geodata_to_image()
{
	QImage terrain_image("./map/terrain.png");
	QImage province_image("./map/provinces.png");

	this->write_terrain_geodata_to_image(terrain_image);
	this->write_province_geodata_to_image(province_image, terrain_image);

	terrain_image.save(QString::fromStdString((database::get_cache_path() / "terrain.png").string()));
	province_image.save(QString::fromStdString((database::get_cache_path() / "provinces.png").string()));
}

/**
**	@brief	Write terrain geodata to the terrain image, caching the result
**
**	@param	terrain_image	The terrain image to be written to
*/
void map::write_terrain_geodata_to_image(QImage &terrain_image)
{
	int processed_terrain_types = 0;

	for (terrain_type *terrain_type : terrain_type::get_all()) {
		const int progress_percent = processed_terrain_types * 100 / static_cast<int>(terrain_type::get_all().size());
		EngineInterface::get()->set_loading_message("Writing Terrain to Image... (" + QString::number(progress_percent) + "%)");

		terrain_type->write_geodata_to_image(terrain_image);

		processed_terrain_types++;
	}
}

/**
**	@brief	Write province geodata to the province image, caching the result
**
**	@param	terrain_image	The terrain image to be written to, for terrain that is written from province data
*/
void map::write_province_geodata_to_image(QImage &province_image, QImage &terrain_image)
{
	std::vector<province *> provinces = province::get_all();
	std::sort(provinces.begin(), provinces.end(), [](const province *a, const province *b) {
		if (a->is_ocean() != b->is_ocean()) {
			return a->is_ocean();
		}

		return a < b;
	});

	int processed_provinces = 0;

	std::set<QRgb> province_image_rgbs = util::get_image_rgbs(province_image);

	for (province *province : provinces) {
		const int progress_percent = processed_provinces * 100 / static_cast<int>(province::get_all().size());
		EngineInterface::get()->set_loading_message("Writing Provinces to Image... (" + QString::number(progress_percent) + "%)");

		if (!province_image_rgbs.contains(province->get_color().rgb()) || province->always_writes_geodata()) {
			province->write_geodata_to_image(province_image, terrain_image);
		}

		processed_provinces++;
	}

	//write river endpoints, but only after everything else, as they should have lower priority than the rivers themselves
	for (province *province : province::get_river_provinces()) {
		if (!province_image_rgbs.contains(province->get_color().rgb()) || province->always_writes_geodata()) {
			province->write_geopath_endpoints_to_image(province_image, terrain_image);
		}
	}
}

}
