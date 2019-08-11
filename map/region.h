#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QVariant>

#include <vector>

namespace Metternich {

class Holding;
class Province;

class Region : public DataEntry, public DataType<Region>
{
	Q_OBJECT

	Q_PROPERTY(QVariantList provinces READ GetProvincesQVariantList)
	Q_PROPERTY(int population READ GetPopulation WRITE SetPopulation NOTIFY PopulationChanged)

public:
	static constexpr const char *ClassIdentifier = "region";
	static constexpr const char *DatabaseFolder = "regions";
	static constexpr const char *Prefix = "r_";

	static Region *Add(const std::string &identifier)
	{
		if (identifier.substr(0, 2) != Region::Prefix) {
			throw std::runtime_error("Invalid identifier for new region: \"" + identifier + "\". Region identifiers must begin with \"" + Region::Prefix + "\".");
		}

		return DataType<Region>::Add(identifier);
	}

public:
	Region(const std::string &identifier) : DataEntry(identifier) {}

	const std::vector<Province *> &GetProvinces() const
	{
		return this->Provinces;
	}

	QVariantList GetProvincesQVariantList() const;

	Q_INVOKABLE void AddProvince(Province *province);
	Q_INVOKABLE void RemoveProvince(Province *province);

	std::vector<Holding *> GetHoldings() const;

	int GetPopulation() const
	{
		return this->Population;
	}

	void SetPopulation(const int population)
	{
		if (population == this->GetPopulation()) {
			return;
		}

		this->Population = population;
		emit PopulationChanged();
	}

	void ChangePopulation(const int change)
	{
		this->SetPopulation(this->GetPopulation() + change);
	}

signals:
	void ProvincesChanged();
	void PopulationChanged();

private:
	std::vector<Province *> Provinces;
	int Population = 0; //the sum of the population of all of the region's provinces
};

}
