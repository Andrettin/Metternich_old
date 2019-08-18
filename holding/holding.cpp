#include "holding.h"

#include "culture/culture.h"
#include "culture/culture_group.h"
#include "economy/commodity.h"
#include "engine_interface.h"
#include "game/game.h"
#include "landed_title/landed_title.h"
#include "map/province.h"
#include "population/population_type.h"
#include "population/population_unit.h"
#include "random.h"
#include "religion.h"
#include "translator.h"
#include "util.h"

#include <utility>

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
**	@brief	Initialize the holding's history
*/
void Holding::InitializeHistory()
{
	if (this->GetCommodity() == nullptr) {
		//generate a commodity for the holding if it produces none
		this->GenerateCommodity();
	}

	for (const std::unique_ptr<PopulationUnit> &population_unit : this->GetPopulationUnits()) {
		//set the culture and religion of population units without any set to those of the holding's province
		if (population_unit->GetCulture() == nullptr) {
			population_unit->SetCulture(this->GetProvince()->GetCulture());
		}

		if (population_unit->GetReligion() == nullptr) {
			population_unit->SetReligion(this->GetProvince()->GetReligion());
		}
	}

	//remove population units with 0 size
	for (size_t i = 0; i < this->PopulationUnits.size();) {
		const std::unique_ptr<PopulationUnit> &population_unit = this->PopulationUnits[i];
		if (population_unit->GetSize() == 0) {
			this->PopulationUnits.erase(this->PopulationUnits.begin() + static_cast<int>(i));
		} else {
			++i;
		}
	}

	this->SortPopulationUnits();
	this->CalculatePopulation();
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
**	@brief	Add a population unit to the holding
*/
void Holding::AddPopulationUnit(std::unique_ptr<PopulationUnit> &&population_unit)
{
	this->ChangePopulation(population_unit->GetSize());
	this->PopulationUnits.push_back(std::move(population_unit));
	emit PopulationUnitsChanged();
}

/**
**	@brief	Get the holding's population units as a QVariantList
**
**	@return	The population units as a QVariantList
*/
QVariantList Holding::GetPopulationUnitsQVariantList() const
{
	QVariantList list;

	for (const std::unique_ptr<PopulationUnit> &population_unit : this->GetPopulationUnits()) {
		list.append(QVariant::fromValue(population_unit.get()));
	}

	return list;
}

/**
**	@brief	Sort the holding's population units
*/
void Holding::SortPopulationUnits()
{
	std::sort(this->PopulationUnits.begin(), this->PopulationUnits.end(), [](const std::unique_ptr<PopulationUnit> &a, const std::unique_ptr<PopulationUnit> &b) {
		//give priority to population units with greater size, so that they will be displayed first
		return a->GetSize() > b->GetSize();
	});

	emit PopulationUnitsChanged();
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

	//change the population count for the province as well
	const int population_change = population - old_population;
	this->GetProvince()->ChangePopulation(population_change);
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
**	@brief	Generate a commodity for the holding to produce
*/
void Holding::GenerateCommodity()
{
	std::map<Metternich::Commodity *, std::pair<int, int>> commodity_chance_ranges;
	int total_chance_factor = 0;
	for (Metternich::Commodity *commodity : Commodity::GetAll()) {
		const int commodity_chance = commodity->CalculateChance(this);
		if (commodity_chance > 0) {
			commodity_chance_ranges[commodity] = std::pair<int, int>(total_chance_factor, total_chance_factor + commodity_chance);
			total_chance_factor += commodity_chance;
		}
	}

	if (commodity_chance_ranges.empty()) {
		return;
	}

	Metternich::Commodity *chosen_commodity = nullptr;

	const int random_number = Random::Generate(total_chance_factor);
	for (const auto &element : commodity_chance_ranges) {
		Metternich::Commodity *commodity = element.first;
		const std::pair<int, int> range = element.second;
		if (random_number >= range.first && random_number < range.second) {
			chosen_commodity = commodity;
		}
	}

	this->SetCommodity(chosen_commodity);
}

/**
**	@brief	Set whether the holding is selected
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
