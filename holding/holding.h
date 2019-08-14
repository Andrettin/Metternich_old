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
	Q_PROPERTY(QVariantList population_units READ GetPopulationUnitsQVariantList NOTIFY PopulationUnitsChanged)
	Q_PROPERTY(Metternich::Commodity* commodity READ GetCommodity WRITE SetCommodity NOTIFY CommodityChanged)
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
	~Holding();

	void Initialize();

	LandedTitle *GetBarony() const
	{
		return this->Barony;
	}

	std::string GetName() const;

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

	bool IsSelected() const
	{
		return this->Selected;
	}

	void SetSelected(const bool selected, const bool notify = true);

signals:
	void NameChanged();
	void TypeChanged();
	void PopulationUnitsChanged();
	void PopulationChanged();
	void CommodityChanged();
	void SelectedChanged();

private:
	LandedTitle *Barony = nullptr;
	HoldingType *Type = nullptr;
	Metternich::Province *Province = nullptr; //the province to which this holding belongs
	std::vector<std::unique_ptr<PopulationUnit>> PopulationUnits;
	int Population = 0; //this holding's population size
	std::set<Building *> Buildings;
	Metternich::Commodity *Commodity = nullptr; //the commodity produced by the holding (if any)
	bool Selected = false;
};

}
