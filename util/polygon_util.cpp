#include "util/polygon_util.h"

#include "util/point_util.h"

namespace metternich::polygon {

QPolygonF from_radius(const double radius, const int angle_per_point, const QPointF &offset)
{
	//create a circle polygon from a given radius
	QPolygonF circle_polygon;
	for (int angle = 0; angle < 360; angle += angle_per_point) {
		const QPointF direction = point::get_degree_angle_direction(angle);
		circle_polygon.append(direction * radius);
	}

	circle_polygon.append(circle_polygon.back()); //close the polygon
	circle_polygon.translate(offset); //apply the offset to it
	return circle_polygon;
}

}
