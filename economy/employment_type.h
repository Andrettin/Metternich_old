#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <map>

namespace metternich {

class Commodity;
class gsml_data;
class Employee;
class EmploymentOwner;
class PopulationType;

class EmploymentType : public DataEntry, public DataType<EmploymentType>
{
	Q_OBJECT

	Q_PROPERTY(int workforce MEMBER Workforce READ GetWorkforce)
	Q_PROPERTY(metternich::Commodity* output_commodity MEMBER OutputCommodity READ GetOutputCommodity)
	Q_PROPERTY(int output_value MEMBER OutputValue READ GetOutputValue)

public:
	static constexpr const char *ClassIdentifier = "employment_type";
	static constexpr const char *DatabaseFolder = "employment_types";

public:
	EmploymentType(const std::string &identifier);
	virtual ~EmploymentType() override;

	virtual void ProcessGSMLProperty(const gsml_property &property) override;
	virtual void ProcessGSMLScope(const gsml_data &scope) override;
	virtual void Initialize() override;

	int GetWorkforce() const
	{
		return this->Workforce;
	}

	Commodity *GetOutputCommodity() const
	{
		return this->OutputCommodity;
	}

	int GetOutputValue() const
	{
		return this->OutputValue;
	}

private:
	EmploymentType *Template = nullptr;
	std::vector<std::unique_ptr<Employee>> Employees;
	std::vector<std::unique_ptr<EmploymentOwner>> Owners;
	int Workforce = 0;
	Commodity *OutputCommodity = nullptr;
	int OutputValue = 0; //how much of the output commodity is produced
	std::map<Commodity *, int> InputCommodities; //the input commodities, mapped to how much of them is necessary for the production
};

}
