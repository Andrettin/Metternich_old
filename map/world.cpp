#include "map/world.h"

#include "database/gsml_data.h"
#include "economy/trade_node.h"
#include "economy/trade_route.h"
#include "game/engine_interface.h"
#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "landed_title/landed_title.h"
#include "map/map.h"
#include "map/pathfinder.h"
#include "map/province.h"
#include "map/star_system.h"
#include "map/terrain_type.h"
#include "map/world_type.h"
#include "util/container_util.h"
#include "util/geocoordinate_util.h"
#include "util/image_util.h"
#include "util/point_util.h"
#include "util/random.h"
#include "util/translator.h"
#include "util/vector_util.h"

namespace metternich {

std::set<std::string> world::get_database_dependencies()
{
	return {
		//so that baronies will be ensured to exist when worlds (and thus holding slots) are processed
		landed_title::class_identifier,
	};
}

world::world(const std::string &identifier) : territory(identifier)
{
}

world::~world()
{
}

void world::initialize()
{
	if (this->terrain_image.size() != this->province_image.size()) {
		throw std::runtime_error("The terrain and province images of world \"" + this->get_identifier() + "\" have different sizes.");
	}

	//clear the terrain and province images, as there is no need to keep them in memory
	this->terrain_image = QImage();
	this->province_image = QImage();

	this->pathfinder = std::make_unique<metternich::pathfinder>(this->provinces);

	if (this->get_star_system() == nullptr) {
		const world *orbit_center = this->get_orbit_center();
		while (orbit_center != nullptr) {
			if (orbit_center->get_star_system() != nullptr) {
				this->set_star_system(orbit_center->get_star_system());
				break;
			}

			orbit_center = orbit_center->get_orbit_center();
		}
	}

	if (this->get_orbit_center() != nullptr) {
		this->set_orbit_angle(random::generate_degree_angle());
	}
	this->calculate_cosmic_size();
	this->set_rotation(random::generate_degree_angle());

	for (world *satellite : this->satellites) {
		if (!satellite->is_initialized()) {
			satellite->initialize();
		}
	}

	const auto satellite_sort_func = [](const world *a, const world *b) {
		if (a->get_distance_from_orbit_center() == 0 || b->get_distance_from_orbit_center() == 0) {
			return a->get_distance_from_orbit_center() != 0; //place satellites without a predefined distance from orbit center last
		}

		return a->get_distance_from_orbit_center() < b->get_distance_from_orbit_center();
	};
	std::sort(this->satellites.begin(), this->satellites.end(), satellite_sort_func);

	//update the satellite distances from this world so that orbits are within a minimum and maximum distance of each other
	const world *previous_satellite = nullptr;
	for (world *satellite : this->satellites) {
		double base_distance = 0;
		if (previous_satellite != nullptr) {
			base_distance += previous_satellite->get_distance_from_orbit_center();
			base_distance += previous_satellite->get_cosmic_size_with_satellites() / 2;
		} else {
			base_distance += this->get_cosmic_size() / 2;
		}

		base_distance += satellite->get_cosmic_size_with_satellites() / 2;
		const double min_distance = base_distance + world::min_orbit_distance;
		const double max_distance = base_distance + world::max_orbit_distance;
		double distance = sqrt(satellite->get_distance_from_orbit_center()) * 10;
		distance = std::max(distance, min_distance);
		distance = std::min(distance, max_distance);
		satellite->set_distance_from_orbit_center(distance);

		previous_satellite = satellite;
	}

	std::sort(this->satellites.begin(), this->satellites.end(), satellite_sort_func);

	connect(this, &world::owner_changed, this, &identifiable_data_entry_base::name_changed); //if the owner changes, whether the world uses its own tag list suffix or that of its system can change

	if (this->get_star_system() != nullptr) {
		connect(this->get_star_system(), &star_system::name_changed, this, &identifiable_data_entry_base::name_changed); //if the tag suffix list of the star system changes, the world's can, too
	}

	territory::initialize();
}

void world::do_day()
{
	if constexpr (world::revolution_enabled) {
		//move the world along its orbit
		if (this->get_orbit_center() != nullptr) {
			this->increment_orbit_angle();
		}
	}

	//rotate the world on its axis
	if (!this->is_star()) { //stars don't need to rotate, as it makes no difference for them graphically
		this->rotate();
	}

	territory::do_day();
}

std::string world::get_name() const
{
	std::vector<std::vector<std::string>> tag_list_with_fallbacks;
	if (this->get_owner() != nullptr) {
		tag_list_with_fallbacks = this->get_tag_suffix_list_with_fallbacks();
	} else if (this->get_star_system() != nullptr) {
		tag_list_with_fallbacks = this->get_star_system()->get_tag_suffix_list_with_fallbacks();
	}

	if (this->get_county() != nullptr) {
		return translator::get()->translate(this->get_county()->get_identifier_with_aliases(), tag_list_with_fallbacks);
	}

	return translator::get()->translate(this->get_identifier_with_aliases(), tag_list_with_fallbacks); //world without a cosmic landed title
}

void world::set_county(landed_title *county)
{
	if (county == this->get_county()) {
		return;
	}

	territory::set_county(county);
	county->set_world(this);
}

void world::set_star_system(metternich::star_system *system)
{
	if (system == this->get_star_system()) {
		return;
	}

	if (this->get_star_system() != nullptr) {
		this->get_star_system()->remove_world(this);
	}

	this->star_system = system;

	if (this->get_star_system() != nullptr) {
		this->get_star_system()->add_world(this);
	}

	emit star_system_changed();
}

const std::filesystem::path &world::get_texture_path() const
{
	//returns a random texture for the world
	return database::get()->get_tagged_texture_path(this->get_type()->get_texture_tag(), {{this->get_identifier()}});
}

void world::set_orbit_angle(const double angle)
{
	if (angle == this->orbit_angle) {
		return;
	}

	this->orbit_angle = angle;
	this->set_orbit_position(point::get_degree_angle_direction(this->orbit_angle));
}

void world::remove_satellite(world *satellite)
{
	vector::remove(this->satellites, satellite);
}

bool world::is_star() const
{
	return this->get_type()->is_star();
}

void world::set_map(const bool map)
{
	if (map == this->has_map()) {
		return;
	}

	if (this->has_map()) {
		vector::remove(world::map_worlds, this);
		this->map_active = false;
	}

	this->map = map;

	if (this->has_map()) {
		world::map_worlds.push_back(this);
		this->map_active = true;
	}
}

void world::calculate_cosmic_map_pos()
{
	if (this->get_astrocoordinate().isValid()) {
		if (this->get_astrocoordinate() == QGeoCoordinate(0, 0)) {
			this->set_cosmic_map_pos(QPointF(0, 0));
			return;
		}

		QPointF direction_pos = geocoordinate::to_circle_edge_point(this->get_astrocoordinate());
		double astrodistance = this->get_astrodistance();
		astrodistance /= 100.;
		astrodistance = cbrt(astrodistance);
		astrodistance *= world::astrodistance_multiplier;
		const double x = direction_pos.x() * astrodistance;
		const double y = direction_pos.y() * astrodistance;
		const QPointF pos(x, y);

		if (this->get_orbit_center() == nullptr || point::distance_to(pos, this->get_orbit_center()->get_cosmic_map_pos()) >= ((this->get_cosmic_size() / 2) + world::min_orbit_distance)) {
			this->set_cosmic_map_pos(pos);
			return;
		}
	}

	if (this->get_orbit_center() != nullptr) {
		this->set_cosmic_map_pos(this->get_orbit_center()->get_cosmic_map_pos() + QPointF(this->get_orbit_position().x() * this->get_distance_from_orbit_center(), this->get_orbit_position().y() * this->get_distance_from_orbit_center()));
		return;
	}

	this->set_cosmic_map_pos(QPointF(0, 0));
}

void world::calculate_cosmic_size()
{
	double cosmic_size = 0;

	if (this->get_diameter() > 0) {
		cosmic_size = cbrt(this->get_diameter());
	}

	if (cosmic_size == 0.) {
		cosmic_size = this->get_default_cosmic_size();
	}

	if (this->is_star()) {
		if (cosmic_size < world::min_star_size && this->get_orbit_center() == nullptr) {
			cosmic_size = world::min_star_size; //only apply the star minimum size to system primary stars
		} else if (cosmic_size > world::max_star_size) {
			cosmic_size = world::max_star_size;
		}
	} else if (this->is_planet()) {
		if (cosmic_size < world::min_planet_size) {
			cosmic_size = world::min_planet_size;
		}
	} else if (this->is_moon()) {
		if (cosmic_size < world::min_moon_size) {
			cosmic_size = world::min_moon_size;
		}
	}

	this->set_cosmic_size(cosmic_size);
}

void world::add_holding_slot(holding_slot *holding_slot)
{
	holding_slot->set_world(this);
	territory::add_holding_slot(holding_slot);
}

bool world::is_system_capital() const
{
	return this->get_star_system() != nullptr && this->get_star_system()->get_capital_world() == this;
}

QVariantList world::get_provinces_qvariant_list() const
{
	return container::to_qvariant_list(this->get_provinces());
}

QVariantList world::get_trade_nodes_qvariant_list() const
{
	return container::to_qvariant_list(this->trade_nodes);
}

QVariantList world::get_trade_routes_qvariant_list() const
{
	return container::to_qvariant_list(this->trade_routes);
}

QPoint world::get_pixel_pos(const int index) const
{
	return point::from_index(index, this->get_map_size());
}

QPoint world::get_coordinate_pos(const QGeoCoordinate &coordinate) const
{
	return geocoordinate::to_point(coordinate, this->get_lon_per_pixel(), this->get_lat_per_pixel());
}

QPointF world::get_coordinate_posf(const QGeoCoordinate &coordinate) const
{
	return geocoordinate::to_pointf(coordinate, this->get_lon_per_pixel(), this->get_lat_per_pixel());
}

QGeoCoordinate world::get_pixel_pos_coordinate(const QPoint &pixel_pos) const
{
	return point::to_geocoordinate(pixel_pos, this->get_map_size());
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

std::vector<QVariantList> world::parse_geojson_folder(const std::string_view &folder) const
{
	std::vector<QVariantList> geojson_data_list;

	for (const std::filesystem::path &path : database::get()->get_map_paths()) {
		const std::filesystem::path map_path = path / this->get_identifier() / folder;

		if (!std::filesystem::exists(map_path)) {
			continue;
		}

		std::vector<QVariantList> folder_geojson_data_list = map::parse_geojson_folder(map_path);
		vector::merge(geojson_data_list, std::move(folder_geojson_data_list));
	}

	return geojson_data_list;
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
	data.for_each_child([&](const gsml_data &child_data) {
		this->process_terrain_gsml_scope(terrain, child_data);
	});
}

void world::process_terrain_gsml_scope(const terrain_type *terrain, const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "geopolygons") {
		scope.for_each_child([&](const gsml_data &polygon_data) {
			this->terrain_geopolygons[terrain].push_back(polygon_data.to_geopolygon());
		});
	} else if (tag == "geopaths") {
		scope.for_each_child([&](const gsml_data &path_data) {
			QGeoPath geopath = path_data.to_geopath();
			geopath.setWidth(terrain->get_path_width());
			this->terrain_geopaths[terrain].push_back(geopath);
		});
	} else {
		throw std::runtime_error("Invalid scope for terrain map data: \"" + tag + "\".");
	}
}

void world::load_province_map()
{
	const std::filesystem::path province_image_path = this->get_cache_path() / "provinces.png";

	if (!std::filesystem::exists(province_image_path)) {
		return;
	}

	engine_interface::get()->set_loading_message("Loading " + this->get_loading_message_name() + " Provinces... (0%)");

	this->province_image = QImage(QString::fromStdString(province_image_path.string()));

	const int pixel_count = this->province_image.width() * this->province_image.height();

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
			if (previous_pixel_province != pixel_province && previous_pixel_province != nullptr) {
				pixel_province->add_border_province(previous_pixel_province);
				previous_pixel_province->add_border_province(pixel_province);
			}

			if (i > this->province_image.width()) { //second line or below
				//the pixel just above this one
				const int adjacent_pixel_index = i - this->province_image.width();
				const QRgb &previous_vertical_pixel_rgb = rgb_data[adjacent_pixel_index];
				province *previous_vertical_pixel_province = province::get_by_rgb(previous_vertical_pixel_rgb, false);
				if (previous_vertical_pixel_province != pixel_province && previous_vertical_pixel_province != nullptr) {
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

		bool inner_river = false;
		for (const auto &kv_pair : province_terrain_count.second) {
			terrain_type *terrain = kv_pair.first;

			if (terrain != nullptr && terrain->is_river() && !inner_river) {
				inner_river = true;
			}
		}

		province->set_inner_river(inner_river);
	}

	for (province *world_province : this->get_provinces()) {
		//add river crossings
		if (world_province->is_river()) {
			//add all of this province's border provinces to each other
			for (province *border_province : world_province->get_border_provinces()) {
				if (border_province->is_water()) {
					continue;
				}

				for (province *other_border_province : world_province->get_border_provinces()) {
					if (other_border_province->is_water()) {
						continue;
					}

					if (border_province->borders_province(other_border_province)) {
						continue; //already borders the other province directly
					}

					border_province->add_river_crossing(other_border_province);
					other_border_province->add_river_crossing(border_province);
				}
			}
		}
	}

	for (province *province : this->get_provinces()) {
		province->calculate_rect();
		province->calculate_center_pos();

		if (province->get_terrain() == nullptr) {
			province->calculate_terrain();
		}
	}
}

void world::load_terrain_map()
{
	const std::filesystem::path terrain_image_path = this->get_cache_path() / "terrain.png";

	if (!std::filesystem::exists(terrain_image_path)) {
		return;
	}

	engine_interface::get()->set_loading_message("Loading " + this->get_loading_message_name() + " Terrain...");
	this->terrain_image = QImage(QString::fromStdString(terrain_image_path.string()));
	this->map_size = this->terrain_image.size(); //set the world's pixel size to that of its terrain map
}

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

	if (!terrain_image.isNull()) {
		//the terrain image has to be saved after the province image has been written, because provinces can also write to it
		terrain_image.save(QString::fromStdString((this->get_cache_path() / "terrain.png").string()));
	}
}

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
	lon = geocoordinate::longitude_to_pixel_longitude(lon, lon_per_pixel);
	const int start_x = geocoordinate::longitude_to_x(lon, lon_per_pixel);

	double start_lat = bottom_left.latitude();
	start_lat = geocoordinate::latitude_to_pixel_latitude(start_lat, lat_per_pixel);

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

	std::vector<province *> geopath_provinces;

	for (province *province : provinces) {
		const int progress_percent = processed_provinces * 100 / static_cast<int>(province::get_all().size());
		engine_interface::get()->set_loading_message("Writing " + this->get_loading_message_name() + " Provinces to Image... (" + QString::number(progress_percent) + "%)");

		if (!province_image_rgbs.contains(province->get_color().rgb()) || province->always_writes_geodata()) {
			province->write_geodata_to_image(province_image, terrain_image);
		}

		processed_provinces++;

		if (province->is_river()) {
			geopath_provinces.push_back(province);
		}
	}

	//write geopath endpoints, but only after everything else, as they should have lower priority than the geopaths themselves
	for (province *province : geopath_provinces) {
		if (!province_image_rgbs.contains(province->get_color().rgb()) || province->always_writes_geodata()) {
			province->write_geopath_endpoints_to_image(province_image, terrain_image);
		}
	}
}

void world::add_province(province *province)
{
	this->provinces.insert(province);
	province->set_world(this);
}

void world::amalgamate()
{
	std::map<landed_title *, int> realm_counts;
	std::map<holding_type *, int> fort_holding_type_counts;
	std::map<holding_type *, int> university_holding_type_counts;
	std::map<holding_type *, int> hospital_holding_type_counts;
	std::map<landed_title *, int> trading_post_realm_counts;
	std::map<holding_type *, int> trading_post_holding_type_counts;
	std::map<holding_type *, int> factory_holding_type_counts;

	for (province *province : this->get_provinces()) {
		for (holding *settlement_holding : province->get_settlement_holdings()) {
			realm_counts[settlement_holding->get_barony()->get_realm()]++;
		}

		if (province->get_fort_holding() != nullptr) {
			fort_holding_type_counts[province->get_fort_holding()->get_type()]++;
		}
		if (province->get_university_holding() != nullptr) {
			university_holding_type_counts[province->get_university_holding()->get_type()]++;
		}
		if (province->get_hospital_holding() != nullptr) {
			hospital_holding_type_counts[province->get_hospital_holding()->get_type()]++;
		}
		if (province->get_trading_post_holding() != nullptr) {
			trading_post_realm_counts[province->get_trading_post_holding()->get_realm()]++;
			trading_post_holding_type_counts[province->get_trading_post_holding()->get_type()]++;
		}
		if (province->get_factory_holding() != nullptr) {
			factory_holding_type_counts[province->get_factory_holding()->get_type()]++;
		}
	}

	std::set<landed_title *> holding_realms;

	//amalgamate the world's map elements into ones for the cosmic map
	for (holding_slot *settlement_slot : this->get_settlement_holding_slots()) {
		settlement_slot->amalgamate_megalopolis();

		if (settlement_slot->get_holding() != nullptr) {
			holding_realms.insert(settlement_slot->get_holding()->get_barony()->get_realm());
		}
	}

	holding_type *best_type = nullptr;
	int best_count = 0;
	for (const auto &kv_pair : fort_holding_type_counts) {
		holding_type *type = kv_pair.first;
		const int count = kv_pair.second;
		if (count > best_count) {
			best_type = type;
			best_count = count;
		}
	}
	if (best_type != nullptr) {
		this->create_holding(this->get_fort_holding_slot(), best_type);
	}

	best_type = nullptr;
	best_count = 0;
	for (const auto &kv_pair : university_holding_type_counts) {
		holding_type *type = kv_pair.first;
		const int count = kv_pair.second;
		if (count > best_count) {
			best_type = type;
			best_count = count;
		}
	}
	if (best_type != nullptr) {
		this->create_holding(this->get_university_holding_slot(), best_type);
	}

	best_type = nullptr;
	best_count = 0;
	for (const auto &kv_pair : hospital_holding_type_counts) {
		holding_type *type = kv_pair.first;
		const int count = kv_pair.second;
		if (count > best_count) {
			best_type = type;
			best_count = count;
		}
	}
	if (best_type != nullptr) {
		this->create_holding(this->get_hospital_holding_slot(), best_type);
	}

	best_type = nullptr;
	best_count = 0;
	for (const auto &kv_pair : trading_post_holding_type_counts) {
		holding_type *type = kv_pair.first;
		const int count = kv_pair.second;
		if (count > best_count) {
			best_type = type;
			best_count = count;
		}
	}
	if (best_type != nullptr) {
		this->create_holding(this->get_trading_post_holding_slot(), best_type);
	}

	landed_title *best_realm = nullptr;
	best_count = 0;
	for (landed_title *realm : holding_realms) {
		const auto find_iterator = trading_post_realm_counts.find(realm);

		if (find_iterator == trading_post_realm_counts.end()) {
			continue;
		}

		const int count = find_iterator->second;
		if (count > best_count) {
			best_realm = realm;
			best_count = count;
		}
	}
	if (best_realm != nullptr) {
		this->get_trading_post_holding()->set_owner(best_realm->get_holder());
	}

	best_type = nullptr;
	best_count = 0;
	for (const auto &kv_pair : factory_holding_type_counts) {
		holding_type *type = kv_pair.first;
		const int count = kv_pair.second;
		if (count > best_count) {
			best_type = type;
			best_count = count;
		}
	}
	if (best_type != nullptr) {
		this->create_holding(this->get_factory_holding_slot(), best_type);
	}

	best_realm = nullptr;
	best_count = 0;
	for (landed_title *realm : holding_realms) {
		const auto find_iterator = realm_counts.find(realm);

		if (find_iterator == realm_counts.end()) {
			continue;
		}

		const int count = find_iterator->second;
		if (count > best_count) {
			best_realm = realm;
			best_count = count;
		}
	}

	if (best_realm != nullptr) {
		//set this world's capital holding to a megalopolis holding owned by the world county's new owner, preferentially the one which is the megalopolis of the new owner's current capital
		if (best_realm->get_capital_province() != nullptr && best_realm->get_capital_province()->get_megalopolis() != nullptr && best_realm->get_capital_province()->get_megalopolis()->get_holding() != nullptr && best_realm->get_capital_province()->get_megalopolis()->get_holding()->get_owner() == best_realm->get_holder()) {
			this->set_capital_holding_slot(best_realm->get_capital_province()->get_megalopolis());
		} else {
			for (holding *megalopolis: this->get_settlement_holdings()) {
				if (megalopolis->get_owner() == best_realm->get_holder()) {
					this->set_capital_holding_slot(megalopolis->get_slot());
					break;
				}
			}
		}

		this->get_county()->set_holder(best_realm->get_holder());

		//set ownership of the star system as well, if it is not owned by anyone yet
		if (this->get_de_jure_duchy()->get_holder() == nullptr) {
			this->get_de_jure_duchy()->set_holder(best_realm->get_holder());
		}
	}

	for (province *province : this->get_provinces()) {
		province->destroy_settlement_holdings(); //will also cause the destruction of extra holdings
	}

	for (landed_title *title : landed_title::get_all()) {
		if (title->get_capital_province() == nullptr || title->get_capital_province()->get_world() != this) {
			continue;
		}

		if (title->is_active_post_amalgamation()) {
			title->set_capital_world(title->get_capital_province()->get_world());
			title->set_capital_province(nullptr);
			continue;
		}

		if (title->get_holder() != nullptr) {
			//destroy landed titles which should only exist in the world map
			title->set_holder(nullptr);
		}
	}

	//calculate population groups for the world, so that e.g. its plurality culture will have been calculated
	this->calculate_population_groups();
}

}
