#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <map>

namespace metternich {

class commodity;
class employee;
class employment_owner;
class gsml_data;
class PopulationType;

class employment_type : public DataEntry, public DataType<employment_type>
{
	Q_OBJECT

	Q_PROPERTY(int workforce MEMBER workforce READ get_workforce)
	Q_PROPERTY(metternich::commodity* output_commodity MEMBER output_commodity READ get_output_commodity)
	Q_PROPERTY(int output_value MEMBER output_value READ get_output_value)

public:
	static constexpr const char *ClassIdentifier = "employment_type";
	static constexpr const char *DatabaseFolder = "employment_types";

public:
	employment_type(const std::string &identifier);
	virtual ~employment_type() override;

	virtual void ProcessGSMLProperty(const gsml_property &property) override;
	virtual void ProcessGSMLScope(const gsml_data &scope) override;
	virtual void Initialize() override;

	int get_workforce() const
	{
		return this->workforce;
	}

	commodity *get_output_commodity() const
	{
		return this->output_commodity;
	}

	int get_output_value() const
	{
		return this->output_value;
	}

	int get_employee_efficiency(const PopulationType *population_type) const;
	bool can_employ_population_type(const PopulationType *population_type) const;

private:
	employment_type *template_type = nullptr;
	std::vector<std::unique_ptr<employee>> employees;
	std::vector<std::unique_ptr<employment_owner>> owners;
	int workforce = 0;
	commodity *output_commodity = nullptr;
	int output_value = 0; //how much of the output commodity is produced
	std::map<commodity *, int> input_commodities; //the input commodities, mapped to how much of them is necessary for the production
};

}
