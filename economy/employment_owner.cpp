#include "economy/employment_owner.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "population/population_type.h"
#include "util/filesystem_util.h"

namespace metternich {

std::unique_ptr<employment_owner> employment_owner::from_gsml_scope(const gsml_data &scope)
{
	auto owner = std::make_unique<employment_owner>();
	owner->population_type = population_type::get(scope.get_tag());

	database::process_gsml_data(owner, scope);

	return owner;
}

void employment_owner::process_gsml_property(const gsml_property &property)
{
	database::process_gsml_property_for_object(this, property);
}

}
