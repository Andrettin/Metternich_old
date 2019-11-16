#include "map/terrain_type.h"

#include "database/gsml_data.h"
#include "engine_interface.h"
#include "script/modifier.h"
#include "util/location_util.h"
#include "util/point_util.h"

namespace metternich {

/**
**	@brief	Get an instance of the class by the RGB value associated with it
**	@param	rgb	The instance's RGB
**	@param	should_find	Whether it is expected that an instance should be found (i.e. if none is, then it is an error).
**	@return	The instance if found, or null otherwise
*/
terrain_type *terrain_type::get_by_rgb(const QRgb &rgb, const bool should_find)
{
	if (rgb == terrain_type::empty_rgb) {
		return nullptr;
	}

	typename std::map<QRgb, terrain_type *>::const_iterator find_iterator = terrain_type::instances_by_rgb.find(rgb);

	if (find_iterator != terrain_type::instances_by_rgb.end()) {
		return find_iterator->second;
	}

	if (should_find) {
		throw std::runtime_error("No terrain found for RGB value: " + std::to_string(rgb) + ".");
	}

	return nullptr;
}

/**
**	@brief	Constructor
*/
terrain_type::terrain_type(const std::string &identifier) : data_entry(identifier)
{
}

/**
**	@brief	Destructor
*/
terrain_type::~terrain_type()
{
}

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void terrain_type::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "color") {
		if (values.size() != 3) {
			throw std::runtime_error("The \"color\" scope for terrains needs to contain exactly three values!");
		}

		const int red = std::stoi(values.at(0));
		const int green = std::stoi(values.at(1));
		const int blue = std::stoi(values.at(2));
		this->color.setRgb(red, green, blue);
		terrain_type::instances_by_rgb[this->color.rgb()] = this;
	} else if (tag == "modifier") {
		this->modifier = std::make_unique<metternich::modifier>();
		database::process_gsml_data(this->modifier, scope);
	} else if (tag == "geopolygons") {
		for (const gsml_data &polygon_data : scope.get_children()) {
			this->geopolygons.push_back(polygon_data.to_geopolygon());
		}
	} else if (tag == "geopaths") {
		for (const gsml_data &path_data : scope.get_children()) {
			QGeoPath geopath = path_data.to_geopath();
			geopath.setWidth(this->get_path_width());
			this->geopaths.push_back(geopath);
		}
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

/**
**	@brief	Write the terrain type's geodata to a terrain image
**
**	@param	image	The image to which the terrain_type's geodata will be written to
*/
void terrain_type::write_geodata_to_image(QImage &image)
{
	for (const QGeoPolygon &geopolygon : this->geopolygons) {
		this->write_geoshape_to_image(image, geopolygon);
	}

	for (const QGeoPath &geopath : this->geopaths) {
		this->write_geoshape_to_image(image, geopath);
	}
}

/**
**	@brief	Write the terrain type's geopath endpoints to a terrain image
**
**	@param	image	The image to which the province's geodata will be written to
*/
void terrain_type::write_geopath_endpoints_to_image(QImage &image)
{
	const int circle_radius = this->get_path_width() / 2;

	for (const QGeoPath &geopath : this->geopaths) {
		QGeoCircle front_geocircle(geopath.path().front(), circle_radius);
		this->write_geoshape_to_image(image, front_geocircle);

		QGeoCircle back_geocircle(geopath.path().back(), circle_radius);
		this->write_geoshape_to_image(image, back_geocircle);
	}
}

/**
**	@brief	Write a geoshape belonging to the terrain type to an image
**
**	@param	image			The image to which the geoshape is to be written
**	@param	geoshape		The geoshape
*/
void terrain_type::write_geoshape_to_image(QImage &image, const QGeoShape &geoshape)
{
	const QString terrain_loading_message = engine_interface::get()->get_loading_message();

	QRgb rgb = this->get_color().rgb();
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
	const int start_x = util::longitude_to_x(lon, lon_per_pixel);

	double start_lat = bottom_left.latitude();
	start_lat = std::round(start_lat / lat_per_pixel) * lat_per_pixel;

	const int pixel_width = static_cast<int>(std::round((std::abs(top_right.longitude() - bottom_left.longitude())) / lon_per_pixel));
	const bool show_progress = pixel_width >= 512;

	for (; lon <= top_right.longitude(); lon += lon_per_pixel) {
		const int x = util::longitude_to_x(lon, lon_per_pixel);

		for (double lat = start_lat; lat <= top_right.latitude(); lat += lat_per_pixel) {
			QGeoCoordinate coordinate(lat, lon);

			const int y = util::latitude_to_y(lat, lat_per_pixel);
			const int pixel_index = util::point_to_index(x, y, image.size());

			//only write the province to the pixel if it is empty, or if this is a river province and the province to overwrite is not an ocean province
			if (rgb_data[pixel_index] != terrain_type::empty_rgb && (!this->is_river() || terrain_type::get_by_rgb(rgb_data[pixel_index])->is_ocean())) {
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

}
