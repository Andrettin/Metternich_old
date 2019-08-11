#include "holding.h"

#include "culture/culture.h"
#include "culture/culture_group.h"
#include "landed_title/landed_title.h"
#include "map/province.h"
#include "religion.h"
#include "translator.h"

namespace Metternich {

/**
**	@brief	Constructor
**
**	@param	barony		The holding's barony
**	@param	type		The holding's type (e.g. city)
**	@param	province	The province where the holding is located
*/
Holding::Holding(LandedTitle *barony, HoldingType *type, Metternich::Province *province) : DataEntry(barony->GetIdentifier()), Barony(barony), Type(type), Province(province)
{
	barony->SetHolding(this);
}

/**
**	@brief	Destructor
*/
Holding::~Holding()
{
	this->Barony->SetHolding(nullptr);
}

/**
**	@brief	Get the holding's name
**
**	@return	The holding's name
*/
std::string Holding::GetName() const
{
	return Translator::GetInstance()->Translate(this->GetBarony()->GetIdentifier(), {this->GetProvince()->GetCulture()->GetIdentifier(), this->GetProvince()->GetCulture()->GetCultureGroup()->GetIdentifier(), this->GetProvince()->GetReligion()->GetIdentifier()});
}

/**
**	@brief	Set the holding's population
**
**	@param	population	The new population size for the holding
*/
void Holding::SetPopulation(const int population)
{
	if (population == this->GetPopulation()) {
		return;
	}

	const int old_population = this->GetPopulation();
	this->Population = population;
	emit PopulationChanged();

	const int population_change = population - old_population;
	this->GetProvince()->ChangePopulation(population_change);
}

}
