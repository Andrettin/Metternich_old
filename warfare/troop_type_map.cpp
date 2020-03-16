#include "warfare/troop_type_map.h"

#include "warfare/troop_type.h"

namespace metternich {

bool troop_type_compare::operator()(troop_type *a, troop_type *b) const
{
	return a->get_category() < b->get_category();
}

}
