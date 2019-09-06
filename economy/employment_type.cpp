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
EmploymentType::EmploymentType(const std::string &identifier) : DataEntry(identifier)
{
}

/**
**	@brief	Destructor
*/
EmploymentType::~EmploymentType()
{
}

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void EmploymentType::ProcessGSMLProperty(const GSMLProperty &property)
{
	if (property.GetKey() == "template") {
		if (property.GetOperator() == GSMLOperator::Assignment) {
			this->Template = EmploymentType::Get(property.GetValue());
		} else {
			throw std::runtime_error("Only the assignment operator may be used for the \"template\" property.");
		}
	} else {
		DataEntryBase::ProcessGSMLProperty(property);
	}
}

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void EmploymentType::ProcessGSMLScope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "input_commodities") {
		for (const GSMLProperty &property : scope.get_properties()) {
			if (property.GetOperator() != GSMLOperator::Assignment) {
				throw std::runtime_error("Only the assignment operator may be used for properties in the \"" + tag + "\" scope.");
			}

			Commodity *commodity = Commodity::Get(property.GetKey());
			const int input_quantity = std::stoi(property.GetValue());
			this->InputCommodities[commodity] = input_quantity;
		}
	} else if (tag == "employees") {
		for (const gsml_data &employee_scope : scope.get_children()) {
			std::unique_ptr<Employee> employee = Employee::FromGSMLScope(employee_scope);
			this->Employees.push_back(std::move(employee));
		}
	} else if (tag == "owners") {
		for (const gsml_data &owner_scope : scope.get_children()) {
			std::unique_ptr<EmploymentOwner> owner = EmploymentOwner::FromGSMLScope(owner_scope);
			this->Owners.push_back(std::move(owner));
		}
	} else {
		DataEntryBase::ProcessGSMLScope(scope);
	}
}

/**
**	@brief	Initialize the employment type
*/
void EmploymentType::Initialize()
{
	//inherit the data from the template
	if (this->Template != nullptr) {
		if (!this->Template->IsInitialized()) {
			this->Template->Initialize();
		}

		if (this->GetWorkforce() == 0) {
			this->Workforce = this->Template->GetWorkforce();
		}

		if (this->GetOutputCommodity() == nullptr) {
			this->OutputCommodity = this->Template->GetOutputCommodity();
		}

		if (this->GetOutputValue() == 0) {
			this->OutputValue= this->Template->GetOutputValue();
		}

		if (this->Employees.empty()) {
			for (const std::unique_ptr<Employee> &employee : this->Template->Employees) {
				this->Employees.push_back(employee->Duplicate());
			}
		}

		if (this->Owners.empty()) {
			for (const std::unique_ptr<EmploymentOwner> &owner : this->Template->Owners) {
				this->Owners.push_back(owner->Duplicate());
			}
		}
	}

	DataEntryBase::Initialize();
}

}
