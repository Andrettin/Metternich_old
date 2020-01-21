#pragma once

#include <QPoint>
#include <QRect>

namespace metternich::rect {

template <typename function_type>
inline void for_each_perimeter_point(const QRect &rect, const function_type &function)
{
	//call a function for each point in the rect's perimeter
	for (int x = rect.x(); x <= rect.right(); ++x) {
		function(QPoint(x, rect.y()));
		function(QPoint(x, rect.bottom()));
	}

	for (int y = rect.y(); y <= rect.bottom(); ++y) {
		function(QPoint(rect.x(), y));
		function(QPoint(rect.right(), y));
	}
}

template <typename function_type>
inline void for_each_perimeter_point_until(const QRect &rect, const function_type &function)
{
	//call a function for each point in the rect's perimeter, until it returns true
	for (int x = rect.x(); x <= rect.right(); ++x) {
		if (function(QPoint(x, rect.y()))) {
			return;
		}

		if (function(QPoint(x, rect.bottom()))) {
			return;
		}
	}

	for (int y = rect.y(); y <= rect.bottom(); ++y) {
		if (function(QPoint(rect.x(), y))) {
			return;
		}

		if (function(QPoint(rect.right(), y))) {
			return;
		}
	}
}

}
