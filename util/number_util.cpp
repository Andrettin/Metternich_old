#include "util/number_util.h"

#include <boost/math/constants/constants.hpp>

namespace metternich::number {

double degree_to_radian(const double degree)
{
	return degree * boost::math::constants::pi<double>() / 180.;
}

}
