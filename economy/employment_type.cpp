#include "economy/employment_type.h"

#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "economy/commodity.h"
#include "economy/employee.h"
#include "economy/employment_owner.h"

namespace metternich {

/**
**	@brief	Constructor
**
**	@param	identifier	The employment type's string identifier
*/
employment_type::employment_type(const std::string &identifier) : data_entry(identifier)
{
}

/**
**	@brief	Destructor
*/
employment_type::~employment_type()
{
}

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void employment_type::process_gsml_property(const gsml_property &property)
{
	if (property.get_key() == "template") {
		if (property.get_operator() == gsml_operator::assignment) {
			this->template_type = employment_type::Get(property.get_value());
		} else {
			throw std::runtime_error("Only the assignment operator may be used for the \"template\" property.");
		}
	} else {
		data_entry_base::process_gsml_property(property);
	}
}

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void employment_type::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "input_commodities") {
		for (const gsml_property &property : scope.get_properties()) {
			if (property.get_operator() != gsml_operator::assignment) {
				throw std::runtime_error("Only the assignment operator may be used for properties in the \"" + tag + "\" scope.");
			}

			commodity *commodity = commodity::Get(property.get_key());
			const int input_quantity = std::stoi(property.get_value());
			this->input_commodities[commodity] = input_quantity;
		}
	} else if (tag == "employees") {
		for (const gsml_data &employee_scope : scope.get_children()) {
			std::unique_ptr<employee> employee = employee::from_gsml_scope(employee_scope);
			this->employees.push_back(std::move(employee));
		}
	} else if (tag == "owners") {
		for (const gsml_data &owner_scope : scope.get_children()) {
			std::unique_ptr<employment_owner> owner = employment_owner::from_gsml_scope(owner_scope);
			this->owners.push_back(std::move(owner));
		}
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

/**
**	@brief	Initialize the employment type
*/
void employment_type::initialize()
{
	//inherit the data from the template
	if (this->template_type != nullptr) {
		if (!this->template_type->is_initialized()) {
			this->template_type->initialize();
		}

		if (this->get_workforce() == 0) {
			this->workforce = this->template_type->get_workforce();
		}

		if (this->get_output_commodity() == nullptr) {
			this->output_commodity = this->template_type->get_output_commodity();
		}

		if (this->get_output_value() == 0) {
			this->output_value = this->template_type->get_output_value();
		}

		if (this->employees.empty()) {
			for (const std::unique_ptr<employee> &employee : this->template_type->employees) {
				this->employees.push_back(employee->duplicate());
			}
		}

		if (this->owners.empty()) {
			for (const std::unique_ptr<employment_owner> &owner : this->template_type->owners) {
				this->owners.push_back(owner->duplicate());
			}
		}
	}

	data_entry_base::initialize();
}

/**
**	@brief	Get the employee efficiency for a given population type
**
**	@param	population_type	The population type
**
**	@return	The employee efficiency for the population type
*/
int employment_type::get_employee_efficiency(const PopulationType *population_type) const
{
	for (const std::unique_ptr<employee> &employee : this->employees) {
		if (employee->get_population_type() == population_type) {
			return employee->get_efficiency();
		}
	}

	return 0;
}

/**
**	@brief	Get whether the employment type allows employment of a given population type
**
**	@param	population_type	The population type
**
**	@return	True if the employment type allows employing the population type, or false otherwise
*/
bool employment_type::can_employ_population_type(const PopulationType *population_type) const
{
	for (const std::unique_ptr<employee> &employee : this->employees) {
		if (employee->get_population_type() == population_type) {
			return true;
		}
	}

	return false;
}

}
