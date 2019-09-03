#pragma once

#include "database/data_entry.h"

#include <QVariant>

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace Metternich {

class Building;
class Character;
class Commodity;
class Culture;
class HoldingType;
class IdentifiableModifier;
class LandedTitle;
class PopulationType;
class PopulationUnit;
class Province;
class Religion;

class Holding : public DataEntry
{
	Q_OBJECT

	Q_PROPERTY(QString name READ GetNameQString NOTIFY NameChanged)
	Q_PROPERTY(QString titled_name READ GetTitledNameQString NOTIFY TitledNameChanged)
	Q_PROPERTY(Metternich::HoldingType* type READ GetType NOTIFY TypeChanged)
	Q_PROPERTY(Metternich::LandedTitle* barony READ GetBarony CONSTANT)
	Q_PROPERTY(int population READ GetPopulation WRITE SetPopulation NOTIFY PopulationChanged)
	Q_PROPERTY(int population_capacity READ GetPopulationCapacity NOTIFY PopulationCapacityChanged)
	Q_PROPERTY(int population_growth READ GetPopulationGrowth NOTIFY PopulationGrowthChanged)
	Q_PROPERTY(QVariantList population_units READ GetPopulationUnitsQVariantList NOTIFY PopulationUnitsChanged)
	Q_PROPERTY(QVariantList buildings READ GetBuildingsQVariantList NOTIFY BuildingsChanged)
	Q_PROPERTY(QVariantList available_buildings READ GetAvailableBuildingsQVariantList NOTIFY AvailableBuildingsChanged)
	Q_PROPERTY(Metternich::Building* under_construction_building READ GetUnderConstructionBuilding NOTIFY UnderConstructionBuildingChanged)
	Q_PROPERTY(int construction_days READ GetConstructionDays NOTIFY ConstructionDaysChanged)
	Q_PROPERTY(Metternich::Commodity* commodity READ GetCommodity WRITE SetCommodity NOTIFY CommodityChanged)
	Q_PROPERTY(int holding_size READ GetHoldingSize WRITE SetHoldingSize NOTIFY HoldingSizeChanged)
	Q_PROPERTY(bool selected READ IsSelected WRITE SetSelected NOTIFY SelectedChanged)

public:
	static Holding *GetSelectedHolding()
	{
		return Holding::SelectedHolding;
	}

private:
	static inline Holding *SelectedHolding = nullptr;

public:
	Holding(LandedTitle *barony, HoldingType *type, Province *province);
	virtual ~Holding() override;

	virtual void InitializeHistory() override;

	void DoDay();
	void DoMonth();

	LandedTitle *GetBarony() const
	{
		return this->Barony;
	}

	virtual std::string GetName() const override;

	QString GetNameQString() const
	{
		return QString::fromStdString(this->GetName());
	}

	std::string GetTypeName() const;
	std::string GetTitledName() const;

	QString GetTitledNameQString() const
	{
		return QString::fromStdString(this->GetTitledName());
	}

	HoldingType *GetType() const
	{
		return this->Type;
	}

	void SetType(HoldingType *type);

	Character *GetOwner() const
	{
		return this->Owner;
	}

	void SetOwner(Character *character)
	{
		if (character == this->GetOwner()) {
			return;
		}

		this->Owner = character;
		emit OwnerChanged();
	}

	Metternich::Province *GetProvince() const
	{
		return this->Province;
	}

	const std::vector<std::unique_ptr<PopulationUnit>> &GetPopulationUnits() const
	{
		return this->PopulationUnits;
	}

	void AddPopulationUnit(std::unique_ptr<PopulationUnit> &&population_unit);
	QVariantList GetPopulationUnitsQVariantList() const;
	void SortPopulationUnits();
	void RemoveEmptyPopulationUnits();

	int GetPopulation() const
	{
		return this->Population;
	}

	void SetPopulation(const int population);

	void ChangePopulation(const int change)
	{
		this->SetPopulation(this->GetPopulation() + change);
	}

	void CalculatePopulation();

	int GetBasePopulationCapacity() const
	{
		return this->BasePopulationCapacity;
	}

	void SetBasePopulationCapacity(const int base_population_capacity)
	{
		if (base_population_capacity == this->GetBasePopulationCapacity()) {
			return;
		}

		this->BasePopulationCapacity = base_population_capacity;
		this->CalculatePopulationCapacity();
	}

	void ChangeBasePopulationCapacity(const int change)
	{
		this->SetBasePopulationCapacity(this->GetBasePopulationCapacity() + change);
	}

	int GetPopulationCapacityModifier() const
	{
		return this->PopulationCapacityModifier;
	}

	void SetPopulationCapacityModifier(const int population_capacity_modifier)
	{
		if (population_capacity_modifier == this->GetPopulationCapacityModifier()) {
			return;
		}

		this->PopulationCapacityModifier = population_capacity_modifier;
		this->CalculatePopulationCapacity();
	}

	void ChangePopulationCapacityModifier(const int change)
	{
		this->SetPopulationCapacityModifier(this->GetPopulationCapacityModifier() + change);
	}

	int GetPopulationCapacity() const
	{
		return this->PopulationCapacity;
	}

	void SetPopulationCapacity(const int population_capacity)
	{
		if (population_capacity == this->GetPopulationCapacity()) {
			return;
		}

		this->PopulationCapacity = population_capacity;
		emit PopulationCapacityChanged();
		this->CalculatePopulationGrowth(); //population growth depends on the population capacity
	}

	void CalculatePopulationCapacity()
	{
		int population_capacity = this->GetBasePopulationCapacity();
		population_capacity *= this->GetPopulationCapacityModifier();
		population_capacity /= 100;
		population_capacity *= this->GetHoldingSize();
		population_capacity /= 100;
		this->SetPopulationCapacity(population_capacity);
	}

	int GetBasePopulationGrowth() const
	{
		return this->BasePopulationGrowth;
	}

	void SetBasePopulationGrowth(const int base_population_growth)
	{
		if (base_population_growth == this->GetBasePopulationGrowth()) {
			return;
		}

		this->BasePopulationGrowth = base_population_growth;
		this->CalculatePopulationGrowth();
	}

	void ChangeBasePopulationGrowth(const int change)
	{
		this->SetBasePopulationGrowth(this->GetBasePopulationGrowth() + change);
	}


	int GetPopulationGrowth() const
	{
		return this->PopulationGrowth;
	}

	void SetPopulationGrowth(const int population_growth)
	{
		if (population_growth == this->GetPopulationGrowth()) {
			return;
		}

		this->PopulationGrowth = population_growth;
		emit PopulationGrowthChanged();
	}

	void CalculatePopulationGrowth()
	{
		if (this->GetPopulation() == 0) {
			this->SetPopulationGrowth(0);
			return;
		}

		int population_growth = this->GetBasePopulationGrowth();
		if (population_growth > 0 && this->GetPopulation() >= this->GetPopulationCapacity()) {
			population_growth = 0;
		}

		this->SetPopulationGrowth(population_growth);
	}

	void DoPopulationGrowth();
	void CheckOverpopulation();
	void CalculatePopulationProportions();

	const std::set<Building *> &GetBuildings() const
	{
		return this->Buildings;
	}

	QVariantList GetBuildingsQVariantList() const;

	Q_INVOKABLE void AddBuilding(Building *building)
	{
		this->Buildings.insert(building);
		emit BuildingsChanged();
	}

	Q_INVOKABLE void RemoveBuilding(Building *building)
	{
		this->Buildings.erase(building);
		emit BuildingsChanged();
	}

	std::vector<Building *> GetAvailableBuildings() const;
	QVariantList GetAvailableBuildingsQVariantList() const;

	Building *GetUnderConstructionBuilding() const
	{
		return this->UnderConstructionBuilding;
	}

	void SetUnderConstructionBuilding(Building *building);

	int GetConstructionDays() const
	{
		return this->ConstructionDays;
	}

	void SetConstructionDays(const int construction_days)
	{
		if (construction_days == this->GetConstructionDays()) {
			return;
		}

		this->ConstructionDays = construction_days;
		emit ConstructionDaysChanged();
	}

	void ChangeConstructionDays(const int change)
	{
		this->SetConstructionDays(this->GetConstructionDays() + change);
	}

	Metternich::Commodity *GetCommodity() const
	{
		return this->Commodity;
	}

	void SetCommodity(Commodity *commodity)
	{
		if (commodity == this->GetCommodity()) {
			return;
		}

		this->Commodity = commodity;
		emit CommodityChanged();
	}

	void GenerateCommodity();

	int GetHoldingSize() const
	{
		return this->HoldingSize;
	}

	void SetHoldingSize(const int holding_size)
	{
		if (holding_size == this->GetHoldingSize()) {
			return;
		}

		this->HoldingSize = holding_size;
		emit HoldingSizeChanged();
		this->CalculatePopulationCapacity();
	}

	bool IsSelected() const
	{
		return this->Selected;
	}

	void SetSelected(const bool selected, const bool notify_engine_interface = true);

	Q_INVOKABLE QVariantList get_culture_proportions() const;
	Q_INVOKABLE void order_construction(const QVariant &building_variant);

signals:
	void NameChanged();
	void TitledNameChanged();
	void TypeChanged();
	void OwnerChanged();
	void PopulationUnitsChanged();
	void PopulationChanged();
	void PopulationCapacityChanged();
	void PopulationGrowthChanged();
	void populationProportionsChanged();
	void BuildingsChanged();
	void AvailableBuildingsChanged();
	void UnderConstructionBuildingChanged();
	void ConstructionDaysChanged();
	void CommodityChanged();
	void HoldingSizeChanged();
	void SelectedChanged();

private:
	LandedTitle *Barony = nullptr;
	HoldingType *Type = nullptr;
	Character *Owner = nullptr; //the owner of the holding
	Metternich::Province *Province = nullptr; //the province to which this holding belongs
	std::vector<std::unique_ptr<PopulationUnit>> PopulationUnits;
	int BasePopulationCapacity = 0; //the base population capacity
	int PopulationCapacityModifier = 100; //the population capacity modifier
	int PopulationCapacity = 0; //the population capacity
	int Population = 0; //the size of this holding's total population
	int BasePopulationGrowth = 0; //the base population growth
	int PopulationGrowth = 0; //the population growth, in permyriad (per 10,000)
	std::set<Building *> Buildings;
	Building *UnderConstructionBuilding = nullptr; //the building currently under construction
	int ConstructionDays = 0; //the amount of days remaining to construct the building under construction
	Metternich::Commodity *Commodity = nullptr; //the commodity produced by the holding (if any)
	int HoldingSize = 100; //the holding size, which affects population capacity (100 = normal size)
	std::set<IdentifiableModifier *> Modifiers; //modifiers applied to the holding
	bool Selected = false;
	std::map<PopulationType *, long long int> PopulationTypeProportions; //the proportion of the population of each population type, in per myriad
	std::map<Metternich::Culture *, long long int> CultureProportions; //the proportion of the population of each culture, in per myriad
	std::map<Metternich::Religion *, long long int> ReligionProportions; //the proportion of the population of each religion, in per myriad
};

}
