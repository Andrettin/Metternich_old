#pragma once

#include <QPoint>

#include <set>

namespace metternich {

struct point_compare
{
	bool operator()(const QPoint &point, const QPoint &other_point) const;
};

using point_set = std::set<QPoint, point_compare>;

template <typename T>
using point_map = std::map<QPoint, T, point_compare>;

}
