#include "map/map.h"

#include "engine_interface.h"
#include "holding/holding_slot.h"
#include "map/map_mode.h"
#include "map/province.h"
#include "map/star_system.h"
#include "map/terrain_type.h"
#include "map/world.h"
#include "util/filesystem_util.h"
#include "util/geocoordinate_util.h"
#include "util/image_util.h"
#include "util/point_util.h"
#include "util/vector_util.h"

#include <QCryptographicHash>
#include <QGeoRectangle>
#include <QImage>
#include <QJsonDocument>
#include <QRect>
#include <QtLocation/private/qgeojson_p.h>

#include <sstream>

namespace metternich {

class path
{
public:
	static constexpr const char *database_folder = "paths";

	static void process_geojson_feature(const QVariantMap &feature)
	{
		const QVariantMap properties = feature.value("properties").toMap();
		const QString start_province_identifier = properties.value("start_province").toString();
		const QString end_province_identifier = properties.value("end_province").toString();

		province *start_province = province::get(start_province_identifier.toStdString());
		province *end_province = province::get(end_province_identifier.toStdString());

		const QVariantList geopath_list = feature.value("data").toList();

		if (geopath_list.size() > 1) {
			throw std::runtime_error("Tried to process a path GeoJSON feature with more than one geopath (for provinces \"" + start_province->get_identifier() + "\" and \"" + end_province->get_identifier() + "\").");
		}

		const QVariantMap geopath_variant_map = geopath_list.front().toMap();
		const QGeoPath geopath = geopath_variant_map.value("data").value<QGeoPath>();

		const std::pair<province *, province *> province_pair(start_province, end_province);

		if (path::geopaths.contains(province_pair)) {
			throw std::runtime_error("Tried to add a path province geopath for provinces \"" + start_province->get_identifier() + "\" and \"" + end_province->get_identifier() + "\", but another one is already present for them.");
		}

		path::geopaths[province_pair] = geopath;
	}

	static inline std::map<std::pair<province *, province *>, QGeoPath> geopaths;
};

std::vector<QVariantList> map::parse_geojson_folder(const std::filesystem::path &path)
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

//parse the GeoJSON data with world coordinates
std::vector<QVariantList> map::parse_world_geojson_database()
{
	std::vector<QVariantList> geojson_data_list;

	for (const std::filesystem::path &path : database::get()->get_map_paths()) {
		const std::filesystem::path map_path = path / world::database_folder;

		if (!std::filesystem::exists(map_path)) {
			continue;
		}

		std::vector<QVariantList> folder_geojson_data_list = map::parse_geojson_folder(map_path);
		vector::merge(geojson_data_list, std::move(folder_geojson_data_list));
	}

	return geojson_data_list;
}

//process the GeoJSON data with world coordinates
void map::process_world_geojson_database()
{
	const std::vector<QVariantList> geojson_data_list = map::parse_world_geojson_database();

	for (const QVariantList &geojson_data : geojson_data_list) {
		const QVariantMap feature_collection = geojson_data.front().toMap();
		const QVariantList feature_collection_data = feature_collection.value("data").toList();

		for (const QVariant &feature_variant : feature_collection_data) {
			const QVariantMap feature = feature_variant.toMap();
			world::process_geojson_feature(feature);
		}
	}
}

map::map() : mode(map_mode::none)
{
}

void map::load()
{
	const bool cache_valid = this->check_cache();

	if (cache_valid) {
		engine_interface::get()->set_loading_message("Loading Map Cache...");
		province::process_cache();
	} else {
		engine_interface::get()->set_loading_message("Building Map Cache...");

		//clear cache
		std::filesystem::path cache_path = database::get_cache_path();
		std::filesystem::remove_all(cache_path);
		std::filesystem::create_directories(cache_path);
	}

	for (world *world : world::get_map_worlds()) {
		//load map data for provinces
		world->process_province_map_database();
	}

	if (!cache_valid) {
		for (world *world : world::get_map_worlds()) {
			//load map data
			world->process_data_type_map_geojson_database<holding_slot>();
			world->process_terrain_map_database();
			world->process_data_type_map_geojson_database<path>();

			world->write_geodata_to_image();
		}

		for (province *province : province::get_all()) {
			if (province->writes_geojson()) {
				province->write_geojson();
			}
		}
	}

	map::process_world_geojson_database();

	for (world *world : world::get_map_worlds()) {
		world->load_terrain_map();
		world->load_province_map();
	}


	for (const auto &kv_pair : path::geopaths) {
		province *start_province = kv_pair.first.first;
		province *end_province = kv_pair.first.second;
		const QGeoPath &geopath = kv_pair.second;
		const world *world = start_province->get_world();

		std::vector<QPointF> path;
		for (const QGeoCoordinate &geocoordinate : geopath.path()) {
			QPointF path_pos = world->get_coordinate_posf(geocoordinate);

			if (path.empty() || path_pos != path.back()) {
				if (path.empty() && end_province->contains_geocoordinate(geocoordinate) && !start_province->contains_geocoordinate(geocoordinate)) {
					qWarning() << QString::fromStdString("The starting position of the path between the provinces of \"" + start_province->get_identifier() + "\" and \"" + end_province->get_identifier() + "\" is in the end province instead of in the start one.");
				}

				path.push_back(std::move(path_pos));
			}
		}

		std::vector<QPointF> reversed_path = path;
		std::reverse(reversed_path.begin(), reversed_path.end());

		start_province->add_path_pos_list(end_province, std::move(path));
		end_province->add_path_pos_list(start_province, std::move(reversed_path));
	}

	if (!cache_valid) {
		engine_interface::get()->set_loading_message("Saving Cache...");
		this->save_cache();
	}
}

void map::set_mode(const map_mode mode)
{
	if (mode == this->get_mode()) {
		return;
	}

	this->mode = mode;

	for (province *province : province::get_all()) {
		province->update_color_for_map_mode(this->get_mode());
	}

	for (star_system *system : star_system::get_all()) {
		system->update_color_for_map_mode(this->get_mode());
	}

	emit engine_interface::get()->map_mode_changed();
}

void map::calculate_cosmic_map_bounding_rect()
{
	this->cosmic_map_bounding_rect = QRectF(0, 0, 0, 0);

	for (const world *world : world::get_all()) {
		if (world->get_orbit_center() != nullptr) {
			continue;
		}

		const double size = world->get_cosmic_size_with_satellites();
		const QPointF pos = world->get_cosmic_map_pos();
		const QPointF top_left(pos.x() - (size / 2) - map::cosmic_map_boundary_offset, pos.y() - (size / 2) - map::cosmic_map_boundary_offset);
		const QPointF bottom_right(pos.x() + (size / 2) + map::cosmic_map_boundary_offset, pos.y() + (size / 2) + map::cosmic_map_boundary_offset);
		const QRectF world_bounding_rect(top_left, bottom_right);
		this->cosmic_map_bounding_rect = this->cosmic_map_bounding_rect.united(world_bounding_rect);
	}

	//calculate territory polygons for star systems, as those depend on the cosmic bounding rect
	star_system::calculate_territory_polygons();
}

/**
**	@brief	Load GeoJSON files and save their data in GSML files
*/
void map::load_geojson_files()
{
	std::filesystem::path map_path = database::get_map_path();

	if (!std::filesystem::exists(map_path)) {
		return;
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

		gsml_data coordinate_data = gsml_data::from_geocoordinate<map::geocoordinate_precision>(longitude, latitude);
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
		data.print_to_dir(database::get_map_path());
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
		data.print_to_dir(database::get_map_path());
	}

	this->geojson_path_data.clear();
}

bool map::check_cache()
{
	QCryptographicHash hash(QCryptographicHash::Md5);

	for (const std::filesystem::path &map_path : database::get()->get_map_paths()) {
		if (!std::filesystem::exists(map_path)) {
			continue;
		}

		filesystem::add_files_to_checksum(hash, map_path);
	}

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
	const std::string stored_checksum = stored_checksum_data.get_property_value("map");

	if (stored_checksum != this->checksum) {
		return false;
	}

	return true;
}

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

}
