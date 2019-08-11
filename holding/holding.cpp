#include "holding.h"

#include "culture/culture.h"
#include "culture/culture_group.h"
#include "engine_interface.h"
#include "game/game.h"
#include "landed_title/landed_title.h"
#include "map/province.h"
#include "population/population_type.h"
#include "population/population_unit.h"
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

	if (!Game::GetInstance()->IsStarting()) {
		//change the population count for the province as well, unless we are loading history for starting a game, in which case the province's population count is used to set the holding population of holdings without any population data set
		const int population_change = population - old_population;
		this->GetProvince()->ChangePopulation(population_change);
	}
}

/**
**	@brief	Calculate the population size for the holding
*/
void Holding::CalculatePopulation()
{
	int population = 0;
	for (const std::unique_ptr<PopulationUnit> &population_unit : this->GetPopulationUnits()) {
		population += population_unit->GetSize();
	}
	this->SetPopulation(population);
}

/**
**	@brief	Generate population units based on the holding's historical population number
*/
void Holding::GeneratePopulationUnits()
{
	if (this->GetPopulation() == 0) {
		return;
	}

	int population = this->GetPopulation();
	for (const std::unique_ptr<PopulationUnit> &population_unit : this->GetPopulationUnits()) {
		//remove the size of existing population units from the total population
		population -= population_unit->GetSize();
	}

	if (population <= 0) {
		return;
	}

	auto population_unit = std::make_unique<PopulationUnit>(PopulationType::GetDefaultType(), this);
	population_unit->SetSize(population);
	population_unit->SetCulture(this->GetProvince()->GetCulture());
	population_unit->SetReligion(this->GetProvince()->GetReligion());
	this->PopulationUnits.push_back(std::move(population_unit));

	this->CalculatePopulation();
}

/**
**	@brief	Sets whether the holding is selected
**
**	@param	selected	Whether the holding is being selected
**
**	@param	notify		Whether to emit signals indicating the change
*/
void Holding::SetSelected(const bool selected, const bool notify)
{
	if (selected == this->IsSelected()) {
		return;
	}

	if (selected) {
		if (Holding::SelectedHolding != nullptr) {
			Holding::SelectedHolding->SetSelected(false, false);
		}
		Holding::SelectedHolding = this;
	} else {
		Holding::SelectedHolding = nullptr;
	}

	this->Selected = selected;

	if (notify) {
		emit SelectedChanged();
		EngineInterface::GetInstance()->emit SelectedHoldingChanged();
	}
}

}
