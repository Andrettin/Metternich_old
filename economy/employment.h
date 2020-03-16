#pragma once

namespace metternich {

class building_slot;
class employment_type;
class holding;
class population_unit;

class employment
{
public:
	employment(const employment_type *type, building_slot *building_slot)
		: type(type), building_slot(building_slot)
	{
	}

	~employment();

	void do_day();

	const employment_type *get_type() const
	{
		return type;
	}

	holding *get_holding() const;

	int get_workforce() const
	{
		return this->workforce;
	}

	void set_workforce(const int workforce);

	void change_workforce(const int change)
	{
		this->set_workforce(this->get_workforce() + change);
	}

	int get_workforce_capacity() const
	{
		return this->workforce_capacity;
	}

	void set_workforce_capacity(const int capacity);

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

	void set_modifier_multiplier(const int multiplier);
	void calculate_modifier_multiplier();

private:
	const employment_type *type = nullptr; //the employment type
	building_slot *building_slot = nullptr; //the building slot to which the employment pertains
	int workforce = 0; //the current workforce for the employment
	int workforce_capacity = 0; //the maximum workforce for the employment
	std::map<population_unit *, int> employee_sizes; //the employed population units, mapped to the quantity of people each of them has assigned for this employment
	int modifier_multiplier = 0;
};

}
