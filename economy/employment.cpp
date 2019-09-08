#include "economy/employment.h"

#include "economy/employment_type.h"
#include "population/population_type.h"
#include "population/population_unit.h"

namespace metternich {

void employment::set_workforce_capacity(const int capacity)
{
	if (capacity == this->get_workforce_capacity()) {
		return;
	}

	const int old_capacity = this->get_workforce_capacity();
	this->workforce_capacity = capacity;

	if (old_capacity > capacity) {
		//the workforce capacity has decreased, so we have to remove excess employment for the employment type
		this->remove_excess_employees();
	}
}

/**
**	@brief	Set the employee size for a given population unit
**
**	@param	employment_type	The employment type
**	@param	size			The new employment size
*/
void employment::set_employee_size(population_unit *employee, const int size)
{
	if (size == this->get_employee_size(employee)) {
		return;
	}

	if (size < 0) {
		throw std::runtime_error("Tried to set a negative employment size for employment type \"" + this->get_type()->GetIdentifier() + "\" for a population unit of type \"" + employee->get_type()->GetIdentifier() + "\".");
	}

	if (size == 0) {
		this->employee_sizes.erase(employee);
	} else {
		this->employee_sizes[employee] = size;
	}
}

/**
**	@brief	Remove employees in excess of the employment capacity
*/
void employment::remove_excess_employees()
{
	int employment = 0;
	for (const auto &kv_pair : this->employee_sizes) {
		const int size = kv_pair.second;
		employment += size;
	}

	int excess_employment = employment - this->get_workforce_capacity();

	if (excess_employment <= 0) {
		return;
	}

	std::vector<population_unit *> population_units_to_remove;

	for (auto &kv_pair : this->employee_sizes) {
		population_unit *population_unit = kv_pair.first;
		int &pop_current_employment = kv_pair.second;
		const int pop_employment_change = -std::min(pop_current_employment, excess_employment);
		pop_current_employment += pop_employment_change;
		excess_employment += pop_employment_change;
		if (pop_current_employment == 0) {
			population_units_to_remove.push_back(population_unit);
		}
	}

	for (population_unit *population_unit : population_units_to_remove) {
		this->employee_sizes.erase(population_unit);
	}
}

}
