#pragma once

#include "util/point_container.h"

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
	using underlying_type = int;
	const underlying_type dx = point.x() - other_point.x();
	const underlying_type dy = point.y() - other_point.y();
	return static_cast<underlying_type>(sqrt(dx * dx + dy * dy));
}

inline double distance_to(const QPointF &point, const QPointF &other_point)
{
	using underlying_type = double;
	const underlying_type dx = point.x() - other_point.x();
	const underlying_type dy = point.y() - other_point.y();
	return static_cast<underlying_type>(sqrt(dx * dx + dy * dy));
}

inline QPointF direction_to(const QPointF &point, const QPointF &other_point)
{
	const double distance = point::distance_to(point, other_point);
	const double dx = other_point.x() - point.x();
	const double dy = other_point.y() - point.y();
	const double x = dx / distance;
	const double y = dy / distance;
	return QPointF(x, y);
}

inline QPoint get_nearest_point(const QPoint &point, const std::vector<QPoint> &other_points)
{
	QPoint nearest_point(-1, -1);
	int best_distance = 0;

	for (const QPoint &other_point : other_points) {
		if (other_point == point) {
			return other_point;
		}

		const int distance = point::distance_to(point, other_point);
		if (best_distance == 0 || distance < best_distance) {
			nearest_point = other_point;
			best_distance = distance;
		}
	}

	if (best_distance == 0) {
		throw std::runtime_error("No nearest point found in list.");
	}

	return nearest_point;
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

inline QPointF get_circle_point(const QPointF &point, const double source_radius, const double target_radius)
{
	const double x = point.x() * target_radius / source_radius;
	const double y = point.y() * target_radius / source_radius;
	return QPointF(x, y);
}

inline QPointF get_nearest_circle_edge_point(const QPointF &point, const double radius)
{
	const double x = radius * point.x() / sqrt(pow(point.x(), 2) + pow(point.y(), 2));
	const double y = radius * point.y() / sqrt(pow(point.x(), 2) + pow(point.y(), 2));
	return QPointF(x, y);
}

inline QPointF get_radian_angle_direction(const double angle)
{
	return QPointF(std::cos(angle), std::sin(angle));
}

extern QPointF get_degree_angle_direction(const double angle);

inline QGeoCoordinate to_geocoordinate(const QPoint &point, const QSize &area_size)
{
	const double lon = (point.x() - (area_size.width() / 2)) * 180.0 / (area_size.width() / 2);
	const double lat = (point.y() - (area_size.height() / 2)) * 90.0 / (area_size.height() / 2) * -1;
	return QGeoCoordinate(lat, lon);
}

}
