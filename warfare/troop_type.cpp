#include "warfare/troop_type.h"

#include "warfare/troop_category.h"

namespace metternich {

troop_type::troop_type(const std::string &identifier)
	: data_entry(identifier), category(troop_category::none)
{
}

void troop_type::check() const
{
	if (this->get_category() == troop_category::none) {
		throw std::runtime_error("Troop type \"" + this->get_identifier() + "\" has no category.");
	}
}

}
