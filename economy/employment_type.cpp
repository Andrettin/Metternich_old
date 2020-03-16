#include "economy/employment_type.h"

#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "economy/commodity.h"
#include "economy/employee.h"
#include "economy/employment_owner.h"
#include "script/modifier.h"
#include "util/string_util.h"

namespace metternich {

employment_type::employment_type(const std::string &identifier) : data_entry(identifier)
{
}

employment_type::~employment_type()
{
}

void employment_type::process_gsml_property(const gsml_property &property)
{
	if (property.get_key() == "template") {
		if (property.get_operator() == gsml_operator::assignment) {
			this->template_type = employment_type::get(property.get_value());
		} else {
			throw std::runtime_error("Only the assignment operator may be used for the \"template\" property.");
		}
	} else {
		data_entry_base::process_gsml_property(property);
	}
}

void employment_type::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "input_commodities") {
		scope.for_each_property([&](const gsml_property &property) {
			if (property.get_operator() != gsml_operator::assignment) {
				throw std::runtime_error("Only the assignment operator may be used for properties in the \"" + tag + "\" scope.");
			}

			commodity *commodity = commodity::get(property.get_key());
			const int input_quantity = std::stoi(property.get_value());
			this->input_commodities[commodity] = input_quantity;
		});
	} else if (tag == "employees") {
		scope.for_each_child([&](const gsml_data &employee_scope) {
			qunique_ptr<employee> employee = employee::from_gsml_scope(employee_scope);
			this->employees.push_back(std::move(employee));
		});
	} else if (tag == "owners") {
		scope.for_each_child([&](const gsml_data &owner_scope) {
			qunique_ptr<employment_owner> owner = employment_owner::from_gsml_scope(owner_scope);
			this->owners.push_back(std::move(owner));
		});
	} else if (tag == "modifier") {
		this->modifier = std::make_unique<metternich::modifier<holding>>();
		database::process_gsml_data(this->modifier, scope);
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

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
			for (const qunique_ptr<employee> &employee : this->template_type->employees) {
				this->employees.push_back(employee->duplicate());
			}
		}

		if (this->owners.empty()) {
			for (const qunique_ptr<employment_owner> &owner : this->template_type->owners) {
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
int employment_type::get_employee_efficiency(const population_type *population_type) const
{
	for (const qunique_ptr<employee> &employee : this->employees) {
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
bool employment_type::can_employ_population_type(const population_type *population_type) const
{
	for (const qunique_ptr<employee> &employee : this->employees) {
		if (employee->get_population_type() == population_type) {
			return true;
		}
	}

	return false;
}

std::string employment_type::get_string() const
{
	std::string str;

	if (!this->input_commodities.empty()) {
		str += "Input: ";
		bool first = true;
		for (const auto &kv_pair : this->input_commodities) {
			if (first) {
				first = false;
			} else {
				str += ", ";
			}
			str += kv_pair.first->get_name();
		}
	}

	if (this->get_output_commodity() != nullptr) {
		str += "Output: " + this->get_output_commodity()->get_name();
	}

	if (this->get_modifier() != nullptr) {
		if (this->get_workforce() == 1) {
			str += "For every employee:\n";
		} else {
			QLocale english_locale(QLocale::English);
			str += "For every " + english_locale.toString(this->get_workforce()).toStdString() + " employees:\n";
		}
		str += this->get_modifier()->get_string(1);
	}
	return str;
}

QString employment_type::get_string_qstring() const
{
	return string::to_tooltip(this->get_string());
}

}
