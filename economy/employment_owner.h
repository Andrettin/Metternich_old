#pragma once

#include <QObject>

namespace Metternich {

class GSMLData;
class GSMLProperty;
class PopulationType;

class EmploymentOwner : public QObject
{
	Q_OBJECT

	Q_PROPERTY(int output_modifier MEMBER OutputModifier READ GetOutputModifier)
	Q_PROPERTY(int proportion_to_workforce MEMBER ProportionToWorkforce READ GetProportionToWorkforce)
	Q_PROPERTY(int income_share MEMBER IncomeShare READ GetIncomeShare)

public:
	static std::unique_ptr<EmploymentOwner> FromGSMLScope(const GSMLData &scope);

	void ProcessGSMLProperty(const GSMLProperty &property);
	void ProcessGSMLScope(const GSMLData &scope) { Q_UNUSED(scope); }

	std::unique_ptr<EmploymentOwner> Duplicate() const
	{
		auto owner = std::make_unique<EmploymentOwner>();
		owner->PopulationType = this->GetPopulationType();
		owner->OutputModifier = this->GetOutputModifier();
		owner->ProportionToWorkforce = this->GetProportionToWorkforce();
		owner->IncomeShare = this->GetIncomeShare();
		return owner;
	}

	const Metternich::PopulationType *GetPopulationType() const
	{
		return this->PopulationType;
	}

	int GetOutputModifier() const
	{
		return this->OutputModifier;
	}

	int GetProportionToWorkforce() const
	{
		return this->ProportionToWorkforce;
	}

	int GetIncomeShare() const
	{
		return this->IncomeShare;
	}

private:
	const Metternich::PopulationType *PopulationType = nullptr;
	int OutputModifier = 100; //the owner's output modifier in ideal conditions (i.e. the proportion of owners to workforce being exactly as in ProportionToWorkforce)
	int ProportionToWorkforce = 1; //the proportion (in percent) that the owners should have to the workforce to grant the output modifier in full
	int IncomeShare = 50; //the share of income for the employment owner
};

}
