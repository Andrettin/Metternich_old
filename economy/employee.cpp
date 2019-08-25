#include "economy/employee.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "population/population_type.h"
#include "util.h"

namespace Metternich {

/**
**	@brief	Create an employee from a GSML scope
**
**	@param	scope	The GSML scope
*/
std::unique_ptr<Employee> Employee::FromGSMLScope(const GSMLData &scope)
{
	auto employee = std::make_unique<Employee>();
	employee->PopulationType = PopulationType::Get(scope.GetTag());

	Database::ProcessGSMLData(employee, scope);

	return employee;
}

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void Employee::ProcessGSMLProperty(const GSMLProperty &property)
{
	Database::ProcessGSMLPropertyForObject(this, property);
}

}
