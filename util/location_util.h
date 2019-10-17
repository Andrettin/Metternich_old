#pragma once

#include <QGeoCoordinate>
#include <QGeoPolygon>
#include <QGeoRectangle>

namespace metternich::util {

/**
**	@brief	Convert longitude to an x coordinate
**
**	@param	longitude		The longitude
**	@param	lon_per_pixel	The longitude per pixel
**
**	@return	The x coordinate
*/
inline int longitude_to_x(const double longitude, const double lon_per_pixel)
{
	return static_cast<int>(std::round((longitude + 180.0) / lon_per_pixel));
}

/**
**	@brief	Convert latitude to an y coordinate
**
**	@param	latitude		The latitude
**	@param	lat_per_pixel	The latitude per pixel
**
**	@return	The y coordinate
*/
inline int latitude_to_y(const double latitude, const double lat_per_pixel)
{
	return static_cast<int>(std::round((latitude + 90.0) / lat_per_pixel));
}

/**
**	@brief	Convert a geocoordinate to a point
**
**	@param	coordinate		The geocoordinate
**	@param	lon_per_pixel	The longitude per pixel
**	@param	lat_per_pixel	The latitude per pixel
**
**	@return	The point
*/
inline QPoint coordinate_to_point(const QGeoCoordinate &coordinate, const double lon_per_pixel, const double lat_per_pixel)
{
	const int x = longitude_to_x(coordinate.longitude(), lon_per_pixel);
	const int y = latitude_to_y(coordinate.latitude(), lat_per_pixel);
	return QPoint(x, y);
}

/**
**	@brief	Get whether a coordinate is in a georectangle (presuming the rectangle is valid)
**
**	@param	coordinate		The geocoordinate
**	@param	georectangle	The georectangle
**
**	@return	True if the coordinate is in the georectangle, or false otherwise
*/
inline bool is_coordinate_in_georectangle(const QGeoCoordinate &coordinate, const QGeoRectangle &georectangle)
{
	const double lat = coordinate.latitude();
	const double lon = coordinate.longitude();
	const QGeoCoordinate bottom_left = georectangle.bottomLeft();
	const QGeoCoordinate top_right = georectangle.topRight();
	return lat >= bottom_left.latitude() && lat <= top_right.latitude() && lon >= bottom_left.longitude() && lon <= top_right.longitude();
}

/**
**	@brief	Get the inner georectangle of a geopolygon, that is, the rectangle that is entirely inside the polygon
**
**	@param	geopolygon	The geopolygon
**
**	@return	The inner georectangle
*/
inline QGeoRectangle get_inner_georectangle(const QGeoPolygon &geopolygon)
{
	QGeoRectangle inner_georectangle = geopolygon.boundingGeoRectangle();
	const QGeoCoordinate original_bottom_left = inner_georectangle.bottomLeft();
	const QGeoCoordinate original_top_right = inner_georectangle.topRight();
	QGeoCoordinate bottom_left = original_bottom_left;
	QGeoCoordinate top_right = original_top_right;

	for (const QGeoCoordinate &coordinate : geopolygon.path()) {
		const double lat = coordinate.latitude();
		const double lon = coordinate.longitude();

		const double min_lat_diff = std::abs(original_bottom_left.latitude() - lat);
		const double max_lat_diff = std::abs(original_top_right.latitude() - lat);
		const double min_lon_diff = std::abs(original_bottom_left.longitude() - lon);
		const double max_lon_diff = std::abs(original_top_right.longitude() - lon);

		if (min_lat_diff <= max_lat_diff && min_lat_diff <= min_lon_diff && min_lat_diff <= max_lon_diff) {
			if (lat > bottom_left.latitude()) {
				bottom_left.setLatitude(lat);
			}
		} else if (max_lat_diff <= min_lon_diff && max_lat_diff <= max_lon_diff) {
			if (lat < top_right.latitude()) {
				top_right.setLatitude(lat);
			}
		} else if (min_lon_diff <= max_lon_diff) {
			if (lon > bottom_left.longitude()) {
				bottom_left.setLongitude(lon);
			}
		} else {
			if (lon < top_right.longitude()) {
				top_right.setLongitude(lon);
			}
		}
	}

	inner_georectangle.setBottomLeft(bottom_left);
	inner_georectangle.setTopRight(top_right);

	return inner_georectangle;
}

}
