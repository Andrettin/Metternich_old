#pragma once

#include <QPoint>
#include <QSize>

namespace metternich::util {

inline int point_to_index(const QPoint &point, const QSize &size)
{
	return point.x() + point.y() * size.width();
}

inline QPoint index_to_point(const int index, const QSize &size)
{
	return QPoint(index % size.width(), index / size.width());
}

}
