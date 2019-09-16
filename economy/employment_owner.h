#pragma once

#include <QObject>

namespace metternich {

class gsml_data;
class gsml_property;
class PopulationType;

class employment_owner : public QObject
{
	Q_OBJECT

	Q_PROPERTY(int output_modifier MEMBER output_modifier READ get_output_modifier)
	Q_PROPERTY(int proportion_to_workforce MEMBER proportion_to_workforce READ get_proportion_to_workforce)
	Q_PROPERTY(int income_share MEMBER income_share READ get_income_share)

public:
	static std::unique_ptr<employment_owner> from_gsml_scope(const gsml_data &scope);

	void ProcessGSMLProperty(const gsml_property &property);
	void ProcessGSMLScope(const gsml_data &scope) { Q_UNUSED(scope); }

	std::unique_ptr<employment_owner> duplicate() const
	{
		auto owner = std::make_unique<employment_owner>();
		owner->population_type = this->get_population_type();
		owner->output_modifier = this->get_output_modifier();
		owner->proportion_to_workforce = this->get_proportion_to_workforce();
		owner->income_share = this->get_income_share();
		return owner;
	}

	const metternich::PopulationType *get_population_type() const
	{
		return this->population_type;
	}

	int get_output_modifier() const
	{
		return this->output_modifier;
	}

	int get_proportion_to_workforce() const
	{
		return this->proportion_to_workforce;
	}

	int get_income_share() const
	{
		return this->income_share;
	}

private:
	const metternich::PopulationType *population_type = nullptr;
	int output_modifier = 100; //the owner's output modifier in ideal conditions (i.e. the proportion of owners to workforce being exactly as in ProportionToWorkforce)
	int proportion_to_workforce = 1; //the proportion (in percent) that the owners should have to the workforce to grant the output modifier in full
	int income_share = 50; //the share of income for the employment owner
};

}
