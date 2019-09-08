#pragma once

namespace metternich {

class EmploymentType;
class holding;
class population_unit;

class employment
{
public:
	employment(const EmploymentType *type) : type(type) {}

	const EmploymentType *get_type() const
	{
		return type;
	}

	int get_workforce_capacity() const
	{
		return this->workforce_capacity;
	}

	void set_workforce_capacity(const int capacity);

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

	void remove_excess_employees();

private:
	const EmploymentType *type = nullptr; //the employment type
	int workforce_capacity = 0; //the maximum workforce for the employment
	const holding *holding = nullptr; //the holding where the employment is located
	std::map<population_unit *, int> employee_sizes; //the employed population units, mapped to the quantity of people each of them has assigned for this employment
};

}
