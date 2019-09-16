#pragma once

namespace metternich {

class employment_type;
class holding;
class population_unit;

class employment
{
public:
	employment(const employment_type *type) : type(type) {}

	void do_day();

	const employment_type *get_type() const
	{
		return type;
	}

	int get_workforce_capacity() const
	{
		return this->workforce_capacity;
	}

	void set_workforce_capacity(const int capacity);

	int get_workforce() const
	{
		return this->workforce;
	}

	int get_unused_workforce_capacity() const
	{
		return this->get_workforce_capacity() - this->get_workforce();
	}

	int get_employee_size(population_unit *employee) const
	{
		auto find_iterator = this->employee_sizes.find(employee);
		if (find_iterator == this->employee_sizes.end()) {
			return 0;
		}

		return find_iterator->second;
	}

	void set_employee_size(population_unit *employee, const int size);

	void change_employee_size(population_unit *employee, const int change)
	{
		this->set_employee_size(employee, this->get_employee_size(employee) + change);
	}

	bool can_employ_population_unit(const population_unit *population_unit) const;
	void remove_excess_employees();

private:
	const employment_type *type = nullptr; //the employment type
	int workforce = 0; //the current workforce for the employment
	int workforce_capacity = 0; //the maximum workforce for the employment
	const holding *holding = nullptr; //the holding where the employment is located
	std::map<population_unit *, int> employee_sizes; //the employed population units, mapped to the quantity of people each of them has assigned for this employment
};

}
