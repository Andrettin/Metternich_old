#include "population/population_type.h"

#include "holding/holding_type.h"
#include "util/container_util.h"

namespace metternich {

void population_type::check() const
{
	if (!this->get_color().isValid()) {
		throw std::runtime_error("Population type \"" + this->get_identifier() + "\" has no valid color.");
	}
}

QVariantList population_type::get_equivalent_types_qvariant_list() const
{
	return container::to_qvariant_list(this->get_equivalent_types());
}

QVariantList population_type::get_holding_types_qvariant_list() const
{
	return container::to_qvariant_list(this->get_holding_types());
}

}
