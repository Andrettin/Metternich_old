#include "technology/technology_set.h"

#include "technology/technology.h"

namespace metternich {

bool technology_compare::operator()(technology *a, technology *b) const
{
	if (a->requires_technology(b)) {
		return false;
	} else if (b->requires_technology(a)) {
		return true;
	}

	return a->get_identifier() < b->get_identifier();
}

}
