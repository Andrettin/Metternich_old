#pragma once

#include "util/point_set.h"

#include <QPoint>
#include <QSize>

#include <cmath>

namespace metternich::point {

inline int to_index(const int x, const int y, const QSize &size)
{
	return x + y * size.width();
}

inline int to_index(const QPoint &point, const QSize &size)
{
	return point::to_index(point.x(), point.y(), size);
}

inline QPoint from_index(const int index, const QSize &size)
{
	return QPoint(index % size.width(), index / size.width());
}

inline int distance_to(const QPoint &point, const QPoint &other_point)
{
	const int dx = point.x() - other_point.x();
	const int dy = point.y() - other_point.y();
	return static_cast<int>(sqrt(dx * dx + dy * dy));
}

inline QPoint get_best_intermediate_point(const QPoint &point, const QPoint &target_point, const point_set &other_points)
{
	const int distance_to_target_pos = point::distance_to(point, target_point);
	QPoint best_intermediate_point(-1, -1);
	int best_distance = 0;

	for (const QPoint &intermediate_point : other_points) {
		if (point::distance_to(intermediate_point, point) >= distance_to_target_pos || point::distance_to(intermediate_point, target_point) >= distance_to_target_pos) {
			continue; //is not an intermediate point
		}

		const int distance = point::distance_to(point, intermediate_point);
		if (best_distance == 0 || distance < best_distance) {
			best_intermediate_point = intermediate_point;
			best_distance = distance;
		}
	}

	return best_intermediate_point;
}

inline QGeoCoordinate to_geocoordinate(const QPoint &point, const QSize &area_size)
{
	const double lon = (point.x() - (area_size.width() / 2)) * 180.0 / (area_size.width() / 2);
	const double lat = (point.y() - (area_size.height() / 2)) * 90.0 / (area_size.height() / 2) * -1;
	return QGeoCoordinate(lat, lon);
}

}
