#include "population/population_unit_base.h"

#include "map/province.h"
#include "map/world.h"

namespace metternich {

territory *population_unit_base::get_territory() const
{
	if (this->get_province() != nullptr) {
		return this->get_province();
	} else if (this->get_world() != nullptr) {
		return this->get_world();
	}

	return nullptr;
}

}
