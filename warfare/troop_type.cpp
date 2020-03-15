#include "warfare/troop_type.h"

#include "warfare/troop_category.h"

namespace metternich {

troop_type::troop_type(const std::string &identifier)
	: data_entry(identifier), category(troop_category::infantry)
{
}

}
