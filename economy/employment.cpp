#include "economy/employment.h"

#include "economy/commodity.h"
#include "economy/employment_type.h"
#include "population/population_type.h"
#include "population/population_unit.h"

namespace metternich {


/**
**	@brief	Do the employment's daily actions
*/
void employment::do_day()
{
	for (auto &kv_pair : this->employee_sizes) {
		population_unit *population_unit = kv_pair.first;
		const int pop_current_employment = kv_pair.second;

		long long int output = this->get_type()->get_output_value();
		output *= pop_current_employment;
		output /= this->get_type()->get_workforce();
		output *= this->get_type()->get_employee_efficiency(population_unit->get_type());
		output /= 100;

		//add wealth as if the commodity had been sold immediately, this should be updated into a more sophisticated sale system later
		int wealth_change = static_cast<int>(output);
		wealth_change *= this->get_type()->get_output_commodity()->get_base_price();
		wealth_change /= 100;
		population_unit->change_wealth(wealth_change);
	}
}

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

	const int old_size = this->get_employee_size(employee);

	if (size == 0) {
		this->employee_sizes.erase(employee);
	} else {
		this->employee_sizes[employee] = size;
	}

	const int diff = size - old_size;
	this->workforce += diff;
	employee->change_unemployed_size(-diff);
}

bool employment::can_employ_population_unit(const population_unit *population_unit) const
{
	if (this->get_unused_workforce_capacity() == 0) {
		return false;
	}

	if (!this->get_type()->can_employ_population_type(population_unit->get_type())) {
		return false;
	}

	return true;
}

/**
**	@brief	Remove employees in excess of the employment capacity
*/
void employment::remove_excess_employees()
{
	int excess_employment = this->get_workforce() - this->get_workforce_capacity();

	if (excess_employment <= 0) {
		return;
	}

	std::vector<population_unit *> population_units_to_remove;

	for (auto &kv_pair : this->employee_sizes) {
		population_unit *population_unit = kv_pair.first;
		int &pop_current_employment = kv_pair.second;
		const int pop_employment_change = -std::min(pop_current_employment, excess_employment);
		this->workforce += pop_employment_change;
		population_unit->change_unemployed_size(-pop_employment_change);
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
