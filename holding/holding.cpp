#include "holding.h"

#include "culture/culture.h"
#include "culture/culture_group.h"
#include "defines.h"
#include "economy/commodity.h"
#include "engine_interface.h"
#include "game/game.h"
#include "holding/building.h"
#include "holding/holding_type.h"
#include "landed_title/landed_title.h"
#include "map/province.h"
#include "population/population_type.h"
#include "population/population_unit.h"
#include "random.h"
#include "religion.h"
#include "script/identifiable_modifier.h"
#include "script/modifier.h"
#include "translator.h"
#include "util.h"

#include <utility>

namespace metternich {

/**
**	@brief	Constructor
**
**	@param	barony		The holding's barony
**	@param	type		The holding's type (e.g. city)
**	@param	province	The province where the holding is located
*/
Holding::Holding(LandedTitle *barony, HoldingType *type, metternich::Province *province) : DataEntry(barony->GetIdentifier()), Barony(barony), Province(province)
{
	barony->SetHolding(this);
	this->ChangeBasePopulationGrowth(Defines::Get()->GetBasePopulationGrowth());
	this->SetType(type);
	this->SetOwner(barony->GetHolder());
	this->ChangeBasePopulationCapacity(province->GetPopulationCapacityAdditiveModifier());
	this->ChangePopulationCapacityModifier(province->GetPopulationCapacityModifier());
	this->ChangeBasePopulationGrowth(province->GetPopulationGrowthModifier());

	connect(this, &Holding::TypeChanged, this, &Holding::TitledNameChanged);
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

	if (this->GetProvince() != nullptr) {
		if (this->GetCulture() == nullptr) {
			this->SetCulture(this->GetProvince()->GetCulture());
		}

		if (this->GetReligion() == nullptr) {
			this->SetReligion(this->GetProvince()->GetReligion());
		}
	}

	for (const std::unique_ptr<PopulationUnit> &population_unit : this->GetPopulationUnits()) {
		//set the culture and religion of population units without any set to those of the holding's province
		if (population_unit->GetCulture() == nullptr) {
			population_unit->SetCulture(this->GetCulture());
		}

		if (population_unit->GetReligion() == nullptr) {
			population_unit->SetReligion(this->GetReligion());
		}
	}

	this->RemoveEmptyPopulationUnits();
	this->SortPopulationUnits();
	this->CalculatePopulation();
	this->CalculatePopulationGroups();
	this->CheckOverpopulation();
}

/**
**	@brief	Do the holding's daily actions
*/
void Holding::DoDay()
{
	//handle construction
	if (this->GetUnderConstructionBuilding() != nullptr) {
		this->ChangeConstructionDays(-1);
		if (this->GetConstructionDays() <= 0) {
			this->AddBuilding(this->GetUnderConstructionBuilding());
			this->SetUnderConstructionBuilding(nullptr);
		}
	}
}

/**
**	@brief	Do the holding's monthly actions
*/
void Holding::DoMonth()
{
	this->DoPopulationGrowth();
	this->RemoveEmptyPopulationUnits();
	this->CalculatePopulationGroups();
}

/**
**	@brief	Get the holding's name
**
**	@return	The holding's name
*/
std::string Holding::GetName() const
{
	return Translator::Get()->Translate(this->GetBarony()->GetIdentifier(), {this->GetCulture()->GetIdentifier(), this->GetCulture()->GetCultureGroup()->GetIdentifier(), this->GetReligion()->GetIdentifier()});
}

/**
**	@brief	Get the holding's type name
**
**	@return	The holding's type name
*/
std::string Holding::GetTypeName() const
{
	const metternich::Culture *culture = this->GetCulture();
	const metternich::Religion *religion = this->GetReligion();

	std::vector<std::string> suffixes;
	suffixes.push_back(culture->GetIdentifier());
	suffixes.push_back(culture->GetCultureGroup()->GetIdentifier());
	suffixes.push_back(religion->GetIdentifier());

	return Translator::Get()->Translate(this->GetType()->GetIdentifier(), suffixes);
}

/**
**	@brief	Get the holding's titled name
**
**	@return	The holding's titled name
*/
std::string Holding::GetTitledName() const
{
	std::string titled_name = this->GetTypeName() + " of ";
	titled_name += this->GetName();
	return titled_name;
}

/**
**	@brief	Set the holding's type
**
**	@param	type	The holding type
*/
void Holding::SetType(HoldingType *type)
{
	if (type == this->GetType()) {
		return;
	}

	if (this->GetType() != nullptr && this->GetType()->GetModifier() != nullptr) {
		this->GetType()->GetModifier()->Remove(this);
	}

	this->Type = type;

	if (this->GetType() != nullptr && this->GetType()->GetModifier() != nullptr) {
		this->GetType()->GetModifier()->Apply(this);
	}

	emit TypeChanged();
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
**	@brief	Remove population units that have size 0
*/
void Holding::RemoveEmptyPopulationUnits()
{
	for (size_t i = 0; i < this->PopulationUnits.size();) {
		const std::unique_ptr<PopulationUnit> &population_unit = this->PopulationUnits[i];
		if (population_unit->GetSize() == 0) {
			this->PopulationUnits.erase(this->PopulationUnits.begin() + static_cast<int>(i));
		} else {
			++i;
		}
	}
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

	this->CalculatePopulationGrowth(); //population growth depends on the current population

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
**	@brief	Do the holding's population growth
*/
void Holding::DoPopulationGrowth()
{
	const int population_growth = this->GetPopulationGrowth();

	if (population_growth == 0) {
		return;
	}

	for (const std::unique_ptr<PopulationUnit> &population_unit : this->GetPopulationUnits()) {
		const int population_capacity_difference = this->GetPopulationCapacity() - this->GetPopulation();

		int change = population_unit->GetSize() * population_growth / 10000;
		if (change == 0) {
			if (population_growth != 0) {
				//if the change is zero but population growth is non-zero, then make a change of 1
				if (population_growth > 0 && population_capacity_difference > 0) {
					change = 1;
				} else {
					change = -1;
				}
			}
		} else if (change > 0 && change > population_capacity_difference) {
			change = population_capacity_difference; //don't grow the population beyond capacity
		}

		population_unit->ChangeSize(change);
	}

	this->CheckOverpopulation();
}

/**
**	@brief	Check if the holding is overpopulated, and if so apply the "overpopulation" modifier
*/
void Holding::CheckOverpopulation()
{
	//give the overpopulation modifier if the holding has become overpopulated, or remove it if it was overpopulated but isn't anymore
	const bool overpopulated = this->GetPopulation() > this->GetPopulationCapacity();
	if (overpopulated) {
		if (this->Modifiers.find(IdentifiableModifier::GetOverpopulationModifier()) == this->Modifiers.end()) {
			IdentifiableModifier::GetOverpopulationModifier()->Apply(this);
			this->Modifiers.insert(IdentifiableModifier::GetOverpopulationModifier());
		}
	} else {
		if (this->Modifiers.find(IdentifiableModifier::GetOverpopulationModifier()) != this->Modifiers.end()) {
			IdentifiableModifier::GetOverpopulationModifier()->Remove(this);
			this->Modifiers.erase(IdentifiableModifier::GetOverpopulationModifier());
		}
	}
}

/**
**	@brief	Calculate the population for each culture, religion and etc.
*/
void Holding::CalculatePopulationGroups()
{
	this->PopulationPerType.clear();
	this->PopulationPerCulture.clear();
	this->PopulationPerReligion.clear();

	for (const std::unique_ptr<PopulationUnit> &population_unit : this->GetPopulationUnits()) {
		this->PopulationPerType[population_unit->GetType()] += population_unit->GetSize();
		this->PopulationPerCulture[population_unit->GetCulture()] += population_unit->GetSize();
		this->PopulationPerReligion[population_unit->GetReligion()] += population_unit->GetSize();
	}

	emit populationGroupsChanged();

	//update the holding's main culture and religion

	metternich::Culture *plurality_culture = nullptr;
	int plurality_culture_size = 0;

	for (const auto &kv_pair : this->PopulationPerCulture) {
		metternich::Culture *culture = kv_pair.first;
		const int culture_size = kv_pair.second;
		if (plurality_culture == nullptr || culture_size > plurality_culture_size) {
			plurality_culture = culture;
			plurality_culture_size = culture_size;
		}
	}

	metternich::Religion *plurality_religion = nullptr;
	int plurality_religion_size = 0;

	for (const auto &kv_pair : this->PopulationPerReligion) {
		metternich::Religion *religion = kv_pair.first;
		const int religion_size = kv_pair.second;
		if (plurality_religion == nullptr || religion_size > plurality_religion_size) {
			plurality_religion = religion;
			plurality_religion_size = religion_size;
		}
	}

	this->SetCulture(plurality_culture);
	this->SetReligion(plurality_religion);
}

/**
**	@brief	Get the holding's buildings as a QVariantList
**
**	@return	The buildings as a QVariantList
*/
QVariantList Holding::GetBuildingsQVariantList() const
{
	return ContainerToQVariantList(this->GetBuildings());
}

void Holding::AddBuilding(Building *building)
{
	if (this->Buildings.find(building) != this->Buildings.end()) {
		throw std::runtime_error("Tried to add the \"" + building->GetIdentifier() + "\" building to a holding that already has it.");
	}

	this->Buildings.insert(building);
	this->ApplyBuildingEffects(building, 1);
	emit BuildingsChanged();
}

void Holding::RemoveBuilding(Building *building)
{
	if (this->Buildings.find(building) == this->Buildings.end()) {
		throw std::runtime_error("Tried to remove the \"" + building->GetIdentifier() + "\" building to a holding that does not have it.");
	}

	this->Buildings.erase(building);
	this->ApplyBuildingEffects(building, -1);
	emit BuildingsChanged();
}

/**
**	@brief	Apply a building's effects to the holding
**
**	@param	building	The building
**	@param	change		The multiplier for the change: 1 to apply, -1 to remove
*/
void Holding::ApplyBuildingEffects(const Building *building, const int change)
{
}

/**
**	@brief	Get the holding's available buildings (including already constructed ones)
**
**	@return	The available buildings
*/
std::vector<Building *> Holding::GetAvailableBuildings() const
{
	std::vector<Building *> available_buildings;

	for (Building *building : this->GetType()->GetBuildings()) {
		if (building->IsAvailableForHolding(this)) {
			available_buildings.push_back(building);
		}
	}

	std::sort(available_buildings.begin(), available_buildings.end(), [this](Building *a, Building *b) {
		//give priority to buildings that have already been built, so that they will be displayed first
		bool a_built = this->GetBuildings().find(a) != this->GetBuildings().end();
		bool b_built = this->GetBuildings().find(b) != this->GetBuildings().end();
		if (a_built != b_built) {
			return a_built;
		}

		return a->GetName() < b->GetName();
	});

	return available_buildings;
}

/**
**	@brief	Get the holding's available buildings as a QVariantList
**
**	@return	The available buildings as a QVariantList
*/
QVariantList Holding::GetAvailableBuildingsQVariantList() const
{
	return ContainerToQVariantList(this->GetAvailableBuildings());
}

/**
**	@brief	Set the under construction building for the holding
**
**	@param	building	The building
*/
void Holding::SetUnderConstructionBuilding(Building *building)
{
	if (building == this->GetUnderConstructionBuilding()) {
		return;
	}

	this->UnderConstructionBuilding = building;
	emit UnderConstructionBuildingChanged();
	if (building != nullptr) {
		this->SetConstructionDays(building->GetConstructionDays());
	}
}

/**
**	@brief	Generate a commodity for the holding to produce
*/
void Holding::GenerateCommodity()
{
	std::map<metternich::Commodity *, std::pair<int, int>> commodity_chance_ranges;
	int total_chance_factor = 0;
	for (metternich::Commodity *commodity : Commodity::GetAll()) {
		const int commodity_chance = commodity->CalculateChance(this);
		if (commodity_chance > 0) {
			commodity_chance_ranges[commodity] = std::pair<int, int>(total_chance_factor, total_chance_factor + commodity_chance);
			total_chance_factor += commodity_chance;
		}
	}

	if (commodity_chance_ranges.empty()) {
		return;
	}

	metternich::Commodity *chosen_commodity = nullptr;

	const int random_number = Random::Generate(total_chance_factor);
	for (const auto &element : commodity_chance_ranges) {
		metternich::Commodity *commodity = element.first;
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
**	@param	notify_engine_interface	Whether to emit a signal notifying the engine interface of the change
*/
void Holding::SetSelected(const bool selected, const bool notify_engine_interface)
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
	emit SelectedChanged();

	if (notify_engine_interface) {
		EngineInterface::Get()->emit selectedHoldingChanged();
	}
}

Q_INVOKABLE QVariantList Holding::get_population_per_type() const
{
	QVariantList population_per_type;

	for (const auto &kv_pair : this->PopulationPerType) {
		QVariantMap type_population;
		type_population["type"] = QVariant::fromValue(kv_pair.first);
		type_population["population"] = QVariant::fromValue(kv_pair.second);
		population_per_type.append(type_population);
	}

	return population_per_type;
}

Q_INVOKABLE QVariantList Holding::get_population_per_culture() const
{
	QVariantList population_per_culture;

	for (const auto &kv_pair : this->PopulationPerCulture) {
		QVariantMap culture_population;
		culture_population["culture"] = QVariant::fromValue(kv_pair.first);
		culture_population["population"] = QVariant::fromValue(kv_pair.second);
		population_per_culture.append(culture_population);
	}

	return population_per_culture;
}

Q_INVOKABLE QVariantList Holding::get_population_per_religion() const
{
	QVariantList population_per_religion;

	for (const auto &kv_pair : this->PopulationPerReligion) {
		QVariantMap religion_population;
		religion_population["religion"] = QVariant::fromValue(kv_pair.first);
		religion_population["population"] = QVariant::fromValue(kv_pair.second);
		population_per_religion.append(religion_population);
	}

	return population_per_religion;
}

void Holding::order_construction(const QVariant &building_variant)
{
	QObject *building_object = qvariant_cast<QObject *>(building_variant);
	Building *building = static_cast<Building *>(building_object);
	Game::Get()->PostOrder([this, building]() {
		this->SetUnderConstructionBuilding(building);
	});
}

}
