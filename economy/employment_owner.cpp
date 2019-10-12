#include "economy/employment_owner.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "population/population_type.h"
#include "util/util.h"

namespace metternich {

/**
**	@brief	Create an employment owner from a GSML scope
**
**	@param	scope	The GSML scope
*/
std::unique_ptr<employment_owner> employment_owner::from_gsml_scope(const gsml_data &scope)
{
	auto owner = std::make_unique<employment_owner>();
	owner->population_type = PopulationType::get(scope.get_tag());

	database::process_gsml_data(owner, scope);

	return owner;
}

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void employment_owner::process_gsml_property(const gsml_property &property)
{
	database::process_gsml_property_for_object(this, property);
}

}
