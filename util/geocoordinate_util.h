#pragma once

#include <QGeoCoordinate>
#include <QGeoPolygon>
#include <QGeoRectangle>

namespace metternich::geocoordinate {

inline double longitude_to_pixel_longitude(const double longitude, const double lon_per_pixel)
{
	//convert longitude to the longitude of the center point of its pixel
	return std::round(longitude / lon_per_pixel) * lon_per_pixel;
}

inline double latitude_to_pixel_latitude(const double latitude, const double lat_per_pixel)
{
	//convert latitude to the latitude of the center point of its pixel
	return std::round(latitude / lat_per_pixel) * lat_per_pixel;
}

inline int longitude_to_x(const double longitude, const double lon_per_pixel)
{
	return static_cast<int>(std::round((longitude + 180.0) / lon_per_pixel));
}

inline int latitude_to_y(const double latitude, const double lat_per_pixel)
{
	return static_cast<int>(std::round((latitude * -1 + 90.0) / lat_per_pixel));
}

inline QPoint to_point(const QGeoCoordinate &coordinate, const double lon_per_pixel, const double lat_per_pixel)
{
	const int x = longitude_to_x(coordinate.longitude(), lon_per_pixel);
	const int y = latitude_to_y(coordinate.latitude(), lat_per_pixel);
	return QPoint(x, y);
}

inline QPointF to_circle_point(const QGeoCoordinate &coordinate)
{
	return QPointF(coordinate.longitude(), coordinate.latitude() * 2);
}

extern QPointF to_circle_edge_point(const QGeoCoordinate &coordinate);

/**
**	@brief	Get whether a coordinate is in a georectangle (presuming the rectangle is valid)
**
**	@param	coordinate		The geocoordinate
**	@param	georectangle	The georectangle
**
**	@return	True if the coordinate is in the georectangle, or false otherwise
*/
inline bool is_in_georectangle(const QGeoCoordinate &coordinate, const QGeoRectangle &georectangle)
{
	const double lat = coordinate.latitude();
	const double lon = coordinate.longitude();
	const QGeoCoordinate bottom_left = georectangle.bottomLeft();
	const QGeoCoordinate top_right = georectangle.topRight();
	return lat >= bottom_left.latitude() && lat <= top_right.latitude() && lon >= bottom_left.longitude() && lon <= top_right.longitude();
}

}
