#include "map/world.h"

#include "database/gsml_data.h"
#include "economy/trade_route.h"
#include "engine_interface.h"
#include "map/province.h"
#include "map/terrain_type.h"
#include "util/container_util.h"
#include "util/image_util.h"
#include "util/location_util.h"
#include "util/point_util.h"

namespace metternich {

QVariantList world::get_provinces_qvariant_list() const
{
	return container::to_qvariant_list(this->get_provinces());
}

QVariantList world::get_trade_routes_qvariant_list() const
{
	return container::to_qvariant_list(this->trade_routes);
}

/**
**	@brief	Convert a pixel index to a pixel position
**
**	@param	index	The index
**
**	@return The pixel position
*/
QPoint world::get_pixel_pos(const int index) const
{
	return point::from_index(index, this->pixel_size);
}

/**
**	@brief	Convert a coordinate to a pixel position on the map
**
**	@param	coordinate	The geocoordinate
**
**	@return The pixel position corresponding to the coordinate
*/
QPoint world::get_coordinate_pos(const QGeoCoordinate &coordinate) const
{
	const double lon_per_pixel = 360.0 / static_cast<double>(this->pixel_size.width());
	const double lat_per_pixel = 180.0 / static_cast<double>(this->pixel_size.height());
	return geocoordinate::to_point(coordinate, lon_per_pixel, lat_per_pixel);
}

/**
**	@brief	Convert a pixel position on the map to a geocoordinate
**
**	@param	pos	The pixel position
**
**	@return The geocoordinate corresponding to the pixel position
*/
QGeoCoordinate world::get_pixel_pos_coordinate(const QPoint &pixel_pos) const
{
	return point::to_geocoordinate(pixel_pos, this->pixel_size);
}

terrain_type *world::get_coordinate_terrain(const QGeoCoordinate &coordinate) const
{
	if (this->terrain_image.isNull()) {
		throw std::runtime_error("Cannot get coordinate terrain after clearing the terrain image from memory.");
	}

	QPoint pos = this->get_coordinate_pos(coordinate);
	QRgb rgb = this->terrain_image.pixel(pos);
	return terrain_type::get_by_rgb(rgb);
}

province *world::get_coordinate_province(const QGeoCoordinate &coordinate) const
{
	if (this->province_image.isNull()) {
		throw std::runtime_error("Cannot get coordinate province after clearing the province image from memory.");
	}

	QPoint pos = this->get_coordinate_pos(coordinate);
	QRgb rgb = this->province_image.pixel(pos);
	return province::get_by_rgb(rgb);
}

void world::process_province_map_database()
{
	if (std::string(province::database_folder).empty()) {
		return;
	}

	std::vector<gsml_data> gsml_map_data_to_process = this->parse_data_type_map_database<province>();

	for (gsml_data &data : gsml_map_data_to_process) {
		province *province = province::get(data.get_tag());

		this->add_province(province);

		database::process_gsml_data<metternich::province>(province, data);
	}
}

void world::process_terrain_map_database()
{
	for (const std::filesystem::path &path : database::get()->get_map_paths()) {
		std::filesystem::path map_path = path / this->get_identifier() / "terrain";

		if (!std::filesystem::exists(map_path)) {
			continue;
		}

		std::filesystem::directory_iterator dir_iterator(map_path);

		for (const std::filesystem::directory_entry &dir_entry : dir_iterator) {
			terrain_type *terrain = terrain_type::get(dir_entry.path().stem().string());

			if (dir_entry.is_directory()) {
				std::filesystem::recursive_directory_iterator subdir_iterator(dir_entry);

				for (const std::filesystem::directory_entry &subdir_entry : subdir_iterator) {
					if (!subdir_entry.is_regular_file() || subdir_entry.path().extension() != ".txt") {
						continue;
					}

					gsml_parser parser(subdir_entry.path());
					this->process_terrain_gsml_data(terrain, parser.parse());
				}
			} else if (dir_entry.is_regular_file() && dir_entry.path().extension() == ".txt") {
				gsml_parser parser(dir_entry.path());
				this->process_terrain_gsml_data(terrain, parser.parse());
			}
		}
	}
}

void world::process_terrain_gsml_data(const terrain_type *terrain, const gsml_data &data)
{
	for (const gsml_data &child_data : data.get_children()) {
		this->process_terrain_gsml_scope(terrain, child_data);
	}
}

void world::process_terrain_gsml_scope(const terrain_type *terrain, const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "geopolygons") {
		for (const gsml_data &polygon_data : scope.get_children()) {
			this->terrain_geopolygons[terrain].push_back(polygon_data.to_geopolygon());
		}
	} else if (tag == "geopaths") {
		for (const gsml_data &path_data : scope.get_children()) {
			QGeoPath geopath = path_data.to_geopath();
			geopath.setWidth(terrain->get_path_width());
			this->terrain_geopaths[terrain].push_back(geopath);
		}
	} else {
		throw std::runtime_error("Invalid scope for terrain map data: \"" + tag + "\".");
	}
}

void world::load_province_map()
{
	engine_interface::get()->set_loading_message("Loading " + this->get_loading_message_name() + " Provinces... (0%)");

	this->province_image = QImage(QString::fromStdString((this->get_cache_path() / "provinces.png").string()));

	this->pixel_size = this->province_image.size(); //set the world's pixel size to that of its province map
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
			engine_interface::get()->set_loading_message("Loading " + this->get_loading_message_name() + " Provinces... (" + QString::number(progress_percent) + "%)");
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
		this->add_province(province);

		terrain_type *best_terrain = nullptr;
		int best_terrain_count = 0;
		bool inner_river = false;

		for (const auto &kv_pair : province_terrain_count.second) {
			terrain_type *terrain = kv_pair.first;

			if (terrain != nullptr && terrain->is_river() && !inner_river) {
				inner_river = true;
			}

			const int count = kv_pair.second;
			if (count > best_terrain_count) {
				best_terrain = terrain;
				best_terrain_count = count;
			}
		}

		province->set_terrain(best_terrain);
		province->set_inner_river(inner_river);

		if (province->is_river()) {
			this->geopath_provinces.insert(province);
		}
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

void world::load_terrain_map()
{
	engine_interface::get()->set_loading_message("Loading " + this->get_loading_message_name() + " Terrain...");
	this->terrain_image = QImage(QString::fromStdString((this->get_cache_path() / "terrain.png").string()));
}

/**
**	@brief	Write geodata to image files
*/
void world::write_geodata_to_image()
{
	if (!std::filesystem::exists(this->get_cache_path())) {
		std::filesystem::create_directories(this->get_cache_path());
	}

	QImage terrain_image;

	for (const std::filesystem::path &path : database::get()->get_map_paths()) {
		const std::filesystem::path map_path = path / this->get_identifier();

		if (!std::filesystem::exists(map_path)) {
			continue;
		}

		const std::filesystem::path terrain_image_path = map_path / "terrain.png";

		if (!std::filesystem::exists(terrain_image_path)) {
			continue;
		}

		terrain_image =  QImage(terrain_image_path.string().c_str());
		this->write_terrain_geodata_to_image(terrain_image);
		break;
	}

	for (const std::filesystem::path &path : database::get()->get_map_paths()) {
		const std::filesystem::path map_path = path / this->get_identifier();

		if (!std::filesystem::exists(map_path)) {
			continue;
		}

		const std::filesystem::path province_image_path = map_path / "provinces.png";

		if (!std::filesystem::exists(province_image_path)) {
			continue;
		}

		QImage province_image(province_image_path.string().c_str());
		this->write_province_geodata_to_image(province_image, terrain_image);
		province_image.save(QString::fromStdString((this->get_cache_path() / "provinces.png").string()));
		break;
	}

	//the terrain image has to be saved after the province image has been written, because provinces can also write to it
	terrain_image.save(QString::fromStdString((this->get_cache_path() / "terrain.png").string()));
}

/**
**	@brief	Write terrain geodata to the terrain image, caching the result
**
**	@param	terrain_image	The terrain image to be written to
*/
void world::write_terrain_geodata_to_image(QImage &terrain_image)
{
	int progress = 0;
	int max_progress = 0;

	for (const auto &kv_pair : this->terrain_geopolygons) {
		max_progress += static_cast<int>(kv_pair.second.size());
	}

	for (const auto &kv_pair : this->terrain_geopaths) {
		max_progress += static_cast<int>(kv_pair.second.size());
	}

	for (const auto &kv_pair : this->terrain_geopolygons) {
		const terrain_type *terrain = kv_pair.first;
		for (const QGeoPolygon &geopolygon : kv_pair.second) {
			const int progress_percent = progress * 100 / max_progress;
			engine_interface::get()->set_loading_message("Writing " + this->get_loading_message_name() + " Terrain to Image... (" + QString::number(progress_percent) + "%)");
			this->write_terrain_geoshape_to_image(terrain, terrain_image, geopolygon);
			progress++;
		}
	}

	for (const auto &kv_pair : this->terrain_geopaths) {
		const terrain_type *terrain = kv_pair.first;
		for (const QGeoPath &geopath : kv_pair.second) {
			const int progress_percent = progress * 100 / max_progress;
			engine_interface::get()->set_loading_message("Writing " + this->get_loading_message_name() + " Terrain to Image... (" + QString::number(progress_percent) + "%)");
			this->write_terrain_geoshape_to_image(terrain, terrain_image, geopath);
			progress++;
		}
	}

	this->write_terrain_geopath_endpoints_to_image(terrain_image);
}

/**
**	@brief	Write the terrain type's geopath endpoints to a terrain image
**
**	@param	image	The image to which the province's geodata will be written to
*/
void world::write_terrain_geopath_endpoints_to_image(QImage &image)
{
	for (const auto &kv_pair : this->terrain_geopaths) {
		const terrain_type *terrain = kv_pair.first;
		const int circle_radius = terrain->get_path_width() / 2;

		for (const QGeoPath &geopath : kv_pair.second) {
			QGeoCircle front_geocircle(geopath.path().front(), circle_radius);
			this->write_terrain_geoshape_to_image(terrain, image, front_geocircle);

			QGeoCircle back_geocircle(geopath.path().back(), circle_radius);
			this->write_terrain_geoshape_to_image(terrain, image, back_geocircle);
		}
	}
}

/**
**	@brief	Write a geoshape belonging to the terrain type to an image
**
**	@param	image			The image to which the geoshape is to be written
**	@param	geoshape		The geoshape
*/
void world::write_terrain_geoshape_to_image(const terrain_type *terrain, QImage &image, const QGeoShape &geoshape)
{
	const QString terrain_loading_message = engine_interface::get()->get_loading_message();

	QRgb rgb = terrain->get_color().rgb();
	QRgb *rgb_data = reinterpret_cast<QRgb *>(image.bits());

	const double lon_per_pixel = 360.0 / static_cast<double>(image.size().width());
	const double lat_per_pixel = 180.0 / static_cast<double>(image.size().height());

	QGeoRectangle georectangle = geoshape.boundingGeoRectangle();
	QGeoCoordinate bottom_left = georectangle.bottomLeft();
	QGeoCoordinate top_right = georectangle.topRight();

	if (geoshape.type() == QGeoShape::ShapeType::PathType) {
		//increase the bounding rectangle of paths slightly, as otherwise a part of the path's width is cut off
		bottom_left.setLatitude(bottom_left.latitude() - 0.1);
		bottom_left.setLongitude(bottom_left.longitude() - 0.1);
		top_right.setLatitude(top_right.latitude() + 0.1);
		top_right.setLongitude(top_right.longitude() + 0.1);
	}

	double lon = bottom_left.longitude();
	lon = std::round(lon / lon_per_pixel) * lon_per_pixel;
	const int start_x = geocoordinate::longitude_to_x(lon, lon_per_pixel);

	double start_lat = bottom_left.latitude();
	start_lat = std::round(start_lat / lat_per_pixel) * lat_per_pixel;

	const int pixel_width = static_cast<int>(std::round((std::abs(top_right.longitude() - bottom_left.longitude())) / lon_per_pixel));
	const bool show_progress = pixel_width >= 512;

	for (; lon <= top_right.longitude(); lon += lon_per_pixel) {
		const int x = geocoordinate::longitude_to_x(lon, lon_per_pixel);

		for (double lat = start_lat; lat <= top_right.latitude(); lat += lat_per_pixel) {
			QGeoCoordinate coordinate(lat, lon);

			const int y = geocoordinate::latitude_to_y(lat, lat_per_pixel);
			const int pixel_index = point::to_index(x, y, image.size());

			//only write the province to the pixel if it is empty, or if this is a river province and the province to overwrite is not an ocean province
			if (rgb_data[pixel_index] != terrain_type::empty_rgb && (!terrain->is_river() || terrain_type::get_by_rgb(rgb_data[pixel_index])->is_ocean())) {
				continue;
			}

			if (!geoshape.contains(coordinate)) {
				continue;
			}

			rgb_data[pixel_index] = rgb;
		}

		if (show_progress) {
			const int progress_percent = (x - start_x) * 100 / pixel_width;
			engine_interface::get()->set_loading_message(terrain_loading_message + "\nWriting Geoshape to Image... (" + QString::number(progress_percent) + "%)");
		}
	}

	engine_interface::get()->set_loading_message(terrain_loading_message);
}

/**
**	@brief	Write province geodata to the province image, caching the result
**
**	@param	terrain_image	The terrain image to be written to, for terrain that is written from province data
*/
void world::write_province_geodata_to_image(QImage &province_image, QImage &terrain_image)
{
	std::vector<province *> provinces = container::to_vector(this->get_provinces());
	std::sort(provinces.begin(), provinces.end(), [](const province *a, const province *b) {
		if (a->is_ocean() != b->is_ocean()) {
			return a->is_ocean();
		}

		return a < b;
	});

	int processed_provinces = 0;

	std::set<QRgb> province_image_rgbs = image::get_rgbs(province_image);

	for (province *province : provinces) {
		const int progress_percent = processed_provinces * 100 / static_cast<int>(province::get_all().size());
		engine_interface::get()->set_loading_message("Writing " + this->get_loading_message_name() + " Provinces to Image... (" + QString::number(progress_percent) + "%)");

		if (!province_image_rgbs.contains(province->get_color().rgb()) || province->always_writes_geodata()) {
			province->write_geodata_to_image(province_image, terrain_image);
		}

		processed_provinces++;
	}

	//write geopath endpoints, but only after everything else, as they should have lower priority than the geopaths themselves
	for (province *province : this->get_geopath_provinces()) {
		if (!province_image_rgbs.contains(province->get_color().rgb()) || province->always_writes_geodata()) {
			province->write_geopath_endpoints_to_image(province_image, terrain_image);
		}
	}

	this->geopath_provinces.clear();
}

void world::add_province(province *province)
{
	this->provinces.insert(province);
	province->set_world(this);
}

}
