#pragma once

#include "database/data_entry.h"

#include <QVariant>

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace Metternich {

class Building;
class Commodity;
class HoldingType;
class LandedTitle;
class PopulationUnit;
class Province;

class Holding : public DataEntry
{
	Q_OBJECT

	Q_PROPERTY(QString name READ GetNameQString NOTIFY NameChanged)
	Q_PROPERTY(Metternich::HoldingType* type READ GetType NOTIFY TypeChanged)
	Q_PROPERTY(Metternich::LandedTitle* barony READ GetBarony CONSTANT)
	Q_PROPERTY(int population READ GetPopulation WRITE SetPopulation NOTIFY PopulationChanged)
	Q_PROPERTY(int population_capacity READ GetPopulationCapacity NOTIFY PopulationCapacityChanged)
	Q_PROPERTY(int population_growth READ GetPopulationGrowth NOTIFY PopulationGrowthChanged)
	Q_PROPERTY(QVariantList population_units READ GetPopulationUnitsQVariantList NOTIFY PopulationUnitsChanged)
	Q_PROPERTY(Metternich::Commodity* commodity READ GetCommodity WRITE SetCommodity NOTIFY CommodityChanged)
	Q_PROPERTY(int holding_size READ GetHoldingSize WRITE SetHoldingSize NOTIFY HoldingSizeChanged)
	Q_PROPERTY(int life_rating READ GetLifeRating NOTIFY LifeRatingChanged)
	Q_PROPERTY(bool selected READ IsSelected WRITE SetSelected NOTIFY SelectedChanged)

public:
	static constexpr int PopulationCapacityPerLifeRating = 10000;
	static constexpr int BasePopulationGrowthPercentMultiplier = 100;

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

	HoldingType *GetType() const
	{
		return this->Type;
	}

	void SetType(HoldingType *type)
	{
		if (type == this->GetType()) {
			return;
		}

		this->Type = type;
		emit TypeChanged();
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
		int population_capacity = this->GetLifeRating() * Holding::PopulationCapacityPerLifeRating;
		population_capacity *= this->GetHoldingSize();
		population_capacity /= 100;
		this->SetPopulationCapacity(population_capacity);
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

		int population_growth = 0;
		if (this->GetPopulationCapacity() >= this->GetPopulation()) {
			//if capacity is equal to or greater than population, use the normal population growth rate formula
			population_growth = this->GetPopulation();
			population_growth *= 100;
			population_growth /= this->GetPopulationCapacity();
			population_growth -= 100;
			population_growth *= -1;
		} else {
			//use the inverse calculation for negative rates
			population_growth = this->GetPopulationCapacity();
			population_growth *= 100;
			population_growth /= this->GetPopulation();
			population_growth -= 100;
		}
		population_growth *= Holding::BasePopulationGrowthPercentMultiplier; // constant multiplier for population growth
		population_growth /= 100;
		this->SetPopulationGrowth(population_growth);
	}

	void DoPopulationGrowth();

	const std::set<Building *> &GetBuildings() const
	{
		return this->Buildings;
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

	int GetLifeRating() const
	{
		return this->LifeRating;
	}

	void SetLifeRating(const int life_rating)
	{
		if (life_rating == this->GetLifeRating()) {
			return;
		}

		this->LifeRating = life_rating;
		emit LifeRatingChanged();
		this->CalculatePopulationCapacity();
	}

	void ChangeLifeRating(const int change)
	{
		this->SetLifeRating(this->GetLifeRating() + change);
	}

	void CalculateLifeRating();

	bool IsSelected() const
	{
		return this->Selected;
	}

	void SetSelected(const bool selected, const bool notify_engine_interface = true);

signals:
	void NameChanged();
	void TypeChanged();
	void PopulationUnitsChanged();
	void PopulationChanged();
	void PopulationCapacityChanged();
	void PopulationGrowthChanged();
	void CommodityChanged();
	void HoldingSizeChanged();
	void LifeRatingChanged();
	void SelectedChanged();

private:
	LandedTitle *Barony = nullptr;
	HoldingType *Type = nullptr;
	Metternich::Province *Province = nullptr; //the province to which this holding belongs
	std::vector<std::unique_ptr<PopulationUnit>> PopulationUnits;
	int PopulationCapacity = 0; //the population capacity
	int Population = 0; //the size of this holding's total population
	int PopulationGrowth = 0; //the population growth, in permyriad (per 10,000)
	std::set<Building *> Buildings;
	Metternich::Commodity *Commodity = nullptr; //the commodity produced by the holding (if any)
	int HoldingSize = 100; //the holding size, which affects population capacity (100 = normal size)
	int LifeRating = 0; //the holding's life rating, which affects population capacity
	bool Selected = false;
};

}
