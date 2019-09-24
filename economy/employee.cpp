#include "economy/employee.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "population/population_type.h"
#include "util.h"

namespace metternich {

/**
**	@brief	Create an employee from a GSML scope
**
**	@param	scope	The GSML scope
*/
std::unique_ptr<employee> employee::from_gsml_scope(const gsml_data &scope)
{
	auto employee = std::make_unique<metternich::employee>();
	employee->population_type = PopulationType::Get(scope.get_tag());

	Database::ProcessGSMLData(employee, scope);

	return employee;
}

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void employee::process_gsml_property(const gsml_property &property)
{
	Database::ProcessGSMLPropertyForObject(this, property);
}

}
