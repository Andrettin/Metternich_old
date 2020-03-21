#include "technology/technology_compare.h"

#include "technology/technology.h"
#include "technology/technology_area_compare.h"

namespace metternich {

bool technology_compare::operator()(technology *a, technology *b) const
{
	if (a->get_area() != b->get_area()) {
		return technology_area_compare()(a->get_area(), b->get_area());
	}

	const int a_level = a->get_level();
	const int b_level = b->get_level();

	if (a_level != b_level) {
		return a_level < b_level;
	}

	return a->get_identifier() < b->get_identifier();
}

}
