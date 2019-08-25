#include "economy/employment_owner.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "population/population_type.h"
#include "util.h"

namespace Metternich {

/**
**	@brief	Create an employment owner from a GSML scope
**
**	@param	scope	The GSML scope
*/
std::unique_ptr<EmploymentOwner> EmploymentOwner::FromGSMLScope(const GSMLData &scope)
{
	auto owner = std::make_unique<EmploymentOwner>();
	owner->PopulationType = PopulationType::Get(scope.GetTag());

	Database::ProcessGSMLData(owner, scope);

	return owner;
}

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void EmploymentOwner::ProcessGSMLProperty(const GSMLProperty &property)
{
	Database::ProcessGSMLPropertyForObject(this, property);
}

}
