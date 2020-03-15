#pragma once

#include "util/qunique_ptr.h"

#include <QObject>

namespace metternich {

class gsml_data;
class gsml_property;
class population_type;

class employee final : public QObject
{
	Q_OBJECT

	Q_PROPERTY(int efficiency MEMBER efficiency READ get_efficiency)
	Q_PROPERTY(int workforce_proportion MEMBER workforce_proportion READ get_workforce_proportion)
	Q_PROPERTY(int income_share MEMBER income_share READ get_income_share)

public:
	static qunique_ptr<employee> from_gsml_scope(const gsml_data &scope);

	void process_gsml_property(const gsml_property &property);
	void process_gsml_scope(const gsml_data &scope) { Q_UNUSED(scope) }

	qunique_ptr<employee> duplicate() const
	{
		auto employee = make_qunique<metternich::employee>();
		employee->population_type = this->get_population_type();
		employee->efficiency = this->get_efficiency();
		employee->workforce_proportion = this->get_workforce_proportion();
		employee->income_share = this->get_income_share();
		return employee;
	}

	const metternich::population_type *get_population_type() const
	{
		return this->population_type;
	}

	int get_efficiency() const
	{
		return this->efficiency;
	}

	int get_workforce_proportion() const
	{
		return this->workforce_proportion;
	}

	int get_income_share() const
	{
		return this->income_share;
	}

private:
	const metternich::population_type *population_type = nullptr;
	int efficiency = 100; //the employee's efficiency, as a percentage
	int workforce_proportion = 100; //the proportion that the employee can make of the workforce, as a percentage
	int income_share = 50; //the share of income for the employee
};

}
