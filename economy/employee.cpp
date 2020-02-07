#include "economy/employee.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "population/population_type.h"
#include "util/filesystem_util.h"

namespace metternich {

qunique_ptr<employee> employee::from_gsml_scope(const gsml_data &scope)
{
	auto employee = make_qunique<metternich::employee>();
	employee->population_type = population_type::get(scope.get_tag());

	database::process_gsml_data(employee, scope);

	return employee;
}

void employee::process_gsml_property(const gsml_property &property)
{
	database::process_gsml_property_for_object(this, property);
}

}
