#pragma once

#include "database/data_entry.h"

#include <set>
#include <string>

namespace Metternich {

class Building;
class HoldingType;
class LandedTitle;
class Province;

class Holding : public DataEntry
{
	Q_OBJECT

	Q_PROPERTY(QString name READ GetNameQString NOTIFY NameChanged)
	Q_PROPERTY(Metternich::HoldingType* type READ GetType NOTIFY TypeChanged)
	Q_PROPERTY(Metternich::LandedTitle* barony READ GetBarony NOTIFY BaronyChanged)
	Q_PROPERTY(int population READ GetPopulation WRITE SetPopulation NOTIFY PopulationChanged)

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

	Province *GetProvince() const
	{
		return this->Province;
	}

	int GetPopulation() const
	{
		return this->Population;
	}

	void SetPopulation(const int population);

	const std::set<Building *> &GetBuildings() const
	{
		return this->Buildings;
	}

signals:
	void NameChanged();
	void TypeChanged();
	void BaronyChanged();
	void PopulationChanged();

private:
	LandedTitle *Barony = nullptr;
	HoldingType *Type = nullptr;
	Metternich::Province *Province = nullptr; //the province to which this holding belongs
	std::set<Building *> Buildings;
	int Population = 0; //this holding's population size
};

}
