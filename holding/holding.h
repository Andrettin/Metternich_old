#pragma once

#include "database/data_entry.h"

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace Metternich {

class Building;
class HoldingType;
class LandedTitle;
class PopulationUnit;
class Province;

class Holding : public DataEntry
{
	Q_OBJECT

	Q_PROPERTY(QString name READ GetNameQString NOTIFY NameChanged)
	Q_PROPERTY(Metternich::HoldingType* type READ GetType NOTIFY TypeChanged)
	Q_PROPERTY(Metternich::LandedTitle* barony READ GetBarony NOTIFY BaronyChanged)
	Q_PROPERTY(int population READ GetPopulation WRITE SetPopulation NOTIFY PopulationChanged)
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

	int GetPopulation() const
	{
		return this->Population;
	}

	void SetPopulation(const int population);
	void CalculatePopulation();
	void GeneratePopulationUnits();

	const std::set<Building *> &GetBuildings() const
	{
		return this->Buildings;
	}

	bool IsSelected() const
	{
		return this->Selected;
	}

	void SetSelected(const bool selected, const bool notify = true);

signals:
	void NameChanged();
	void TypeChanged();
	void BaronyChanged();
	void PopulationChanged();
	void SelectedChanged();

private:
	LandedTitle *Barony = nullptr;
	HoldingType *Type = nullptr;
	Metternich::Province *Province = nullptr; //the province to which this holding belongs
	std::vector<std::unique_ptr<PopulationUnit>> PopulationUnits;
	int Population = 0; //this holding's population size
	std::set<Building *> Buildings;
	bool Selected = false;
};

}
