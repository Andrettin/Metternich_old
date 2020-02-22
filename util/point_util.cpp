#include "util/point_util.h"

#include "util/number_util.h"

namespace metternich::point {

QPointF get_degree_angle_direction(const double angle)
{
	const double radian_angle = number::degree_to_radian(angle);
	return point::get_radian_angle_direction(radian_angle);
}

}
