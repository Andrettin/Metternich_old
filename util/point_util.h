#pragma once

#include <QPoint>
#include <QSize>

namespace metternich::util {

inline int point_to_index(const int x, const int y, const QSize &size)
{
	return x + y * size.width();
}

inline int point_to_index(const QPoint &point, const QSize &size)
{
	return point_to_index(point.x(), point.y(), size);
}

inline QPoint index_to_point(const int index, const QSize &size)
{
	return QPoint(index % size.width(), index / size.width());
}

}
