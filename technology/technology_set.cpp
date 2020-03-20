#include "technology/technology_set.h"

#include "technology/technology.h"
#include "technology/technology_area.h"

namespace metternich {

bool technology_compare::operator()(technology *a, technology *b) const
{
	if (a->get_category() != b->get_category()) {
		return a->get_category() < b->get_category();
	}

	if (a->get_area() != b->get_area()) {
		const int a_area_level = a->get_area()->get_min_level();
		const int b_area_level = b->get_area()->get_min_level();

		if (a_area_level != b_area_level) {
			return a_area_level < b_area_level;
		}

		return a->get_area()->get_identifier() < b->get_area()->get_identifier();
	}

	const int a_level = a->get_level();
	const int b_level = b->get_level();

	if (a_level != b_level) {
		return a_level < b_level;
	}

	return a->get_identifier() < b->get_identifier();
}

}
