#pragma once

#include "random.h"

#include <boost/math/constants/constants.hpp>

namespace metternich {

double random::generate_angle()
{
	return random::generate_in_range(0., 1.) * 2. * boost::math::constants::pi<double>();
}

QPointF random::generate_circle_position()
{
	const double angle = random::generate_angle();
	return QPointF(std::cos(angle), std::sin(angle));
}

}
