#pragma once

#include <QObject>

namespace metternich {

class gsml_data;
class gsml_property;
class PopulationType;

class Employee : public QObject
{
	Q_OBJECT

	Q_PROPERTY(int efficiency MEMBER Efficiency READ GetEfficiency)
	Q_PROPERTY(int workforce_proportion MEMBER WorkforceProportion READ GetWorkforceProportion)
	Q_PROPERTY(int income_share MEMBER IncomeShare READ GetIncomeShare)

public:
	static std::unique_ptr<Employee> FromGSMLScope(const gsml_data &scope);

	void ProcessGSMLProperty(const gsml_property &property);
	void ProcessGSMLScope(const gsml_data &scope) { Q_UNUSED(scope); }

	std::unique_ptr<Employee> Duplicate() const
	{
		auto employee = std::make_unique<Employee>();
		employee->PopulationType = this->GetPopulationType();
		employee->Efficiency = this->GetEfficiency();
		employee->WorkforceProportion = this->GetWorkforceProportion();
		employee->IncomeShare = this->GetIncomeShare();
		return employee;
	}

	const metternich::PopulationType *GetPopulationType() const
	{
		return this->PopulationType;
	}

	int GetEfficiency() const
	{
		return this->Efficiency;
	}

	int GetWorkforceProportion() const
	{
		return this->WorkforceProportion;
	}

	int GetIncomeShare() const
	{
		return this->IncomeShare;
	}

private:
	const metternich::PopulationType *PopulationType = nullptr;
	int Efficiency = 100; //the employee's efficiency, as a percentage
	int WorkforceProportion = 100; //the proportion that the employee can make of the workforce, as a percentage
	int IncomeShare = 50; //the share of income for the employee
};

}
