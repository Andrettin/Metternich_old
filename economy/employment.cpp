#include "economy/employment.h"

#include "economy/commodity.h"
#include "economy/employment_type.h"
#include "holding/building_slot.h"
#include "population/population_type.h"
#include "population/population_unit.h"
#include "script/modifier.h"

namespace metternich {

employment::~employment()
{
	if (this->modifier_multiplier != 0) {
		this->get_type()->get_modifier()->remove(this->get_holding(), this->modifier_multiplier);
	}
}

void employment::do_day()
{
	if (this->get_type()->get_output_commodity() == nullptr) {
		return;
	}

	for (const auto &kv_pair : this->employee_sizes) {
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

holding *employment::get_holding() const
{
	return this->building_slot->get_holding();
}

void employment::set_workforce(const int workforce)
{
	if (workforce == this->get_workforce()) {
		return;
	}

	this->workforce = workforce;
	emit this->building_slot->workforce_changed();
}

void employment::set_workforce_capacity(const int capacity)
{
	if (capacity == this->get_workforce_capacity()) {
		return;
	}

	const int old_capacity = this->get_workforce_capacity();
	this->workforce_capacity = capacity;
	emit this->building_slot->workforce_capacity_changed();

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
		throw std::runtime_error("Tried to set a negative employment size for employment type \"" + this->get_type()->get_identifier() + "\" for a population unit of type \"" + employee->get_type()->get_identifier() + "\".");
	}

	const int old_size = this->get_employee_size(employee);

	if (size == 0) {
		this->employee_sizes.erase(employee);
		employee->remove_employment(this);
	} else {
		this->employee_sizes[employee] = size;

		if (old_size == 0) {
			employee->add_employment(this);
		}
	}

	const int diff = size - old_size;
	this->change_workforce(diff);
	this->calculate_modifier_multiplier();
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
		this->change_workforce(pop_employment_change);
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

	this->calculate_modifier_multiplier();
}

void employment::set_modifier_multiplier(const int multiplier)
{
	if (multiplier == this->modifier_multiplier) {
		return;
	}

	if (this->modifier_multiplier != 0) {
		this->get_type()->get_modifier()->remove(this->get_holding(), this->modifier_multiplier);
	}

	this->modifier_multiplier = multiplier;

	if (this->modifier_multiplier != 0) {
		this->get_type()->get_modifier()->apply(this->get_holding(), this->modifier_multiplier);
	}
}

void employment::calculate_modifier_multiplier()
{
	if (this->get_type()->get_modifier() == nullptr) {
		return;
	}

	int modifier_percent = 0;

	for (const auto &kv_pair : this->employee_sizes) {
		population_unit *population_unit = kv_pair.first;
		const int pop_current_employment = kv_pair.second;

		long long int pop_modifier_percent = 100;
		pop_modifier_percent *= pop_current_employment;
		pop_modifier_percent /= this->get_type()->get_workforce();
		pop_modifier_percent *= this->get_type()->get_employee_efficiency(population_unit->get_type());
		pop_modifier_percent /= 100;

		modifier_percent += static_cast<int>(pop_modifier_percent);
	}

	this->set_modifier_multiplier(modifier_percent / 100);
}

}
