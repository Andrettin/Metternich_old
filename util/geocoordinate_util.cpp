#include "util/geocoordinate_util.h"

#include "util/point_util.h"

namespace metternich::geocoordinate {

QPointF to_circle_edge_point(const QGeoCoordinate &coordinate)
{
	const QPointF circle_point = geocoordinate::to_circle_point(coordinate);
	const QPointF circle_edge_point = point::get_nearest_circle_edge_point(circle_point, 180.);
	return point::get_circle_point(circle_edge_point, 180., 1.);
}

}
