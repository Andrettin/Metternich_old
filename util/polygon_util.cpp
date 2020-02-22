#include "util/polygon_util.h"

#include "util/point_util.h"

namespace metternich::polygon {

QPolygonF from_radius(const double radius, const QPointF &offset)
{
	//create a circle polygon from a given radius
	QPolygonF circle_polygon;
	for (int angle = 0; angle < 360; ++angle) {
		const QPointF direction = point::get_angle_direction(angle);
		circle_polygon.append(direction * radius);
	}

	std::sort(circle_polygon.begin(), circle_polygon.end(), [](const QPointF &a, const QPointF &b) {
		if (a.y() <= 0) {
			if (b.y() <= 0) {
				return a.x() < b.x();
			} else {
				return true;
			}
		} else {
			if (b.y() <= 0) {
				return false;
			} else {
				return b.x() < a.x();
			}
		}
	});

	circle_polygon.append(circle_polygon.back()); //close the polygon
	circle_polygon.translate(offset); //apply the offset to it
	return circle_polygon;
}

}
