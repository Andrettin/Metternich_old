#include "technology/technology_area_compare.h"

#include "technology/technology_area.h"

namespace metternich {

bool technology_area_compare::operator()(technology_area *a, technology_area *b) const
{
	if (a->get_category() != b->get_category()) {
		return a->get_category() < b->get_category();
	}

	const int a_min_level = a->get_min_level();
	const int b_min_level = b->get_min_level();

	if (a_min_level != b_min_level) {
		return a_min_level < b_min_level;
	}

	return a->get_identifier() < b->get_identifier();
}

}
