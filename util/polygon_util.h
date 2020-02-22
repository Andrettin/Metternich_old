#pragma once

#include <QPolygonF>

namespace metternich::polygon {

inline double get_area(const QPolygonF &polygon)
{
	double area = 0.;

	for (int i = 0; i < polygon.size(); ++i) {
		const QPointF &point = polygon[i];
		const int j = (i + 1) % polygon.size();
		const QPointF &other_point = polygon[j];
		area += 0.5 * (point.x() * other_point.y() -  other_point.x() * point.y());
	}

	return area;
}

extern QPolygonF from_radius(const double radius, const QPointF &offset = QPointF(0, 0));

}
