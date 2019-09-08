#pragma once

#include "database/data_entry.h"

#include <QVariant>

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace metternich {

class Building;
class Character;
class Commodity;
class Culture;
class employment;
class EmploymentType;
class HoldingType;
class IdentifiableModifier;
class LandedTitle;
class PopulationType;
class population_unit;
class Province;
class Religion;

class Holding : public DataEntry
{
	Q_OBJECT

	Q_PROPERTY(QString name READ GetNameQString NOTIFY NameChanged)
	Q_PROPERTY(QString titled_name READ GetTitledNameQString NOTIFY TitledNameChanged)
	Q_PROPERTY(metternich::HoldingType* type READ GetType NOTIFY TypeChanged)
	Q_PROPERTY(metternich::LandedTitle* barony READ GetBarony CONSTANT)
	Q_PROPERTY(int population READ GetPopulation WRITE SetPopulation NOTIFY PopulationChanged)
	Q_PROPERTY(int population_capacity READ GetPopulationCapacity NOTIFY PopulationCapacityChanged)
	Q_PROPERTY(int population_growth READ GetPopulationGrowth NOTIFY PopulationGrowthChanged)
	Q_PROPERTY(QVariantList population_units READ get_population_units_qvariant_list NOTIFY population_units_changed)
	Q_PROPERTY(QVariantList buildings READ GetBuildingsQVariantList NOTIFY BuildingsChanged)
	Q_PROPERTY(QVariantList available_buildings READ GetAvailableBuildingsQVariantList NOTIFY AvailableBuildingsChanged)
	Q_PROPERTY(metternich::Building* under_construction_building READ GetUnderConstructionBuilding NOTIFY UnderConstructionBuildingChanged)
	Q_PROPERTY(int construction_days READ GetConstructionDays NOTIFY ConstructionDaysChanged)
	Q_PROPERTY(metternich::Commodity* commodity READ GetCommodity WRITE SetCommodity NOTIFY CommodityChanged)
	Q_PROPERTY(metternich::Culture* culture READ GetCulture WRITE SetCulture NOTIFY CultureChanged)
	Q_PROPERTY(metternich::Religion* religion READ GetReligion WRITE SetReligion NOTIFY ReligionChanged)
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

	virtual void initialize_history() override;

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

	metternich::Province *GetProvince() const
	{
		return this->Province;
	}

	const std::vector<std::unique_ptr<population_unit>> &get_population_units() const
	{
		return this->population_units;
	}

	void add_population_unit(std::unique_ptr<population_unit> &&population_unit);
	QVariantList get_population_units_qvariant_list() const;
	void sort_population_units();
	void remove_empty_population_units();

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

	const std::map<PopulationType *, int> &GetPopulationPerType() const
	{
		return this->PopulationPerType;
	}

	int GetPopulationTypePopulation(PopulationType *population_type) const
	{
		auto find_iterator = this->PopulationPerType.find(population_type);
		if (find_iterator == this->PopulationPerType.end()) {
			return 0;
		}

		return find_iterator->second;
	}

	const std::map<metternich::Culture *, int> &GetPopulationPerCulture() const
	{
		return this->PopulationPerCulture;
	}

	int GetCulturePopulation(metternich::Culture *culture) const
	{
		auto find_iterator = this->PopulationPerCulture.find(culture);
		if (find_iterator == this->PopulationPerCulture.end()) {
			return 0;
		}

		return find_iterator->second;
	}

	const std::map<metternich::Religion *, int> &GetPopulationPerReligion() const
	{
		return this->PopulationPerReligion;
	}

	int GetReligionPopulation(metternich::Religion *religion) const
	{
		auto find_iterator = this->PopulationPerReligion.find(religion);
		if (find_iterator == this->PopulationPerReligion.end()) {
			return 0;
		}

		return find_iterator->second;
	}

	void CalculatePopulationGroups();

	const std::set<Building *> &GetBuildings() const
	{
		return this->Buildings;
	}

	QVariantList GetBuildingsQVariantList() const;
	Q_INVOKABLE void AddBuilding(Building *building);
	Q_INVOKABLE void RemoveBuilding(Building *building);
	void ApplyBuildingEffects(const Building *building, const int change);
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

	metternich::Commodity *GetCommodity() const
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

	metternich::Culture *GetCulture() const
	{
		return this->Culture;
	}

	void SetCulture(Culture *culture)
	{
		if (culture == this->GetCulture()) {
			return;
		}

		this->Culture = culture;
		emit CultureChanged();
	}

	metternich::Religion *GetReligion() const
	{
		return this->Religion;
	}

	void SetReligion(Religion *religion)
	{
		if (religion == this->GetReligion()) {
			return;
		}

		this->Religion = religion;
		emit ReligionChanged();
	}

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

	int GetEmploymentWorkforce(const EmploymentType *employment_type) const;
	void SetEmploymentWorkforce(const EmploymentType *employment_type, const int workforce);

	void ChangeEmploymentWorkforce(const EmploymentType *employment_type, const int change)
	{
		this->SetEmploymentWorkforce(employment_type, this->GetEmploymentWorkforce(employment_type) + change);
	}

	bool IsSelected() const
	{
		return this->Selected;
	}

	void SetSelected(const bool selected, const bool notify_engine_interface = true);

	Q_INVOKABLE QVariantList get_population_per_type() const;
	Q_INVOKABLE QVariantList get_population_per_culture() const;
	Q_INVOKABLE QVariantList get_population_per_religion() const;
	Q_INVOKABLE void order_construction(const QVariant &building_variant);

signals:
	void NameChanged();
	void TitledNameChanged();
	void TypeChanged();
	void OwnerChanged();
	void population_units_changed();
	void PopulationChanged();
	void PopulationCapacityChanged();
	void PopulationGrowthChanged();
	void populationGroupsChanged();
	void BuildingsChanged();
	void AvailableBuildingsChanged();
	void UnderConstructionBuildingChanged();
	void ConstructionDaysChanged();
	void CommodityChanged();
	void CultureChanged();
	void ReligionChanged();
	void HoldingSizeChanged();
	void SelectedChanged();

private:
	LandedTitle *Barony = nullptr;
	HoldingType *Type = nullptr;
	Character *Owner = nullptr; //the owner of the holding
	metternich::Province *Province = nullptr; //the province to which this holding belongs
	std::vector<std::unique_ptr<population_unit>> population_units;
	int BasePopulationCapacity = 0; //the base population capacity
	int PopulationCapacityModifier = 100; //the population capacity modifier
	int PopulationCapacity = 0; //the population capacity
	int Population = 0; //the size of this holding's total population
	int BasePopulationGrowth = 0; //the base population growth
	int PopulationGrowth = 0; //the population growth, in permyriad (per 10,000)
	std::set<Building *> Buildings;
	Building *UnderConstructionBuilding = nullptr; //the building currently under construction
	int ConstructionDays = 0; //the amount of days remaining to construct the building under construction
	metternich::Commodity *Commodity = nullptr; //the commodity produced by the holding (if any)
	int HoldingSize = 100; //the holding size, which affects population capacity (100 = normal size)
	metternich::Culture *Culture = nullptr; //the holding's culture
	metternich::Religion *Religion = nullptr; //the holding's religion
	std::set<IdentifiableModifier *> Modifiers; //modifiers applied to the holding
	bool Selected = false;
	std::map<const EmploymentType *, std::unique_ptr<employment>> employments; //employments, mapped to their respective employment types
	std::map<PopulationType *, int> PopulationPerType; //the population for each population type
	std::map<metternich::Culture *, int> PopulationPerCulture; //the population for each culture
	std::map<metternich::Religion *, int> PopulationPerReligion; //the population for each religion
};

}
