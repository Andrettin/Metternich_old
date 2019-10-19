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
	return static_cast<int>(std::round((latitude * -1 + 90.0) / lat_per_pixel));
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

}
