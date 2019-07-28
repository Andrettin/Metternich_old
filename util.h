#pragma once

#include <QPoint>
#include <QSize>

#include <string>

inline bool StringToBool(const std::string &str)
{
	if (str == "true" || str == "yes" || str == "1") {
		return true;
	} else if (str == "false" || str == "no" || str == "0") {
		return false;
	}

	throw std::runtime_error("Invalid string used for conversion to boolean: \"" + str + "\".");
}

inline int PointToIndex(const QPoint &point, const QSize &size)
{
	return point.x() + point.y() * size.width();
}

inline QPoint IndexToPoint(const int index, const QSize &size)
{
	return QPoint(index % size.width(), index / size.width());
}
