#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QVariant>

#include <memory>
#include <vector>

namespace Metternich {

class Holding;
class PopulationUnit;
class Province;

class Region : public DataEntry, public DataType<Region>
{
	Q_OBJECT

	Q_PROPERTY(QVariantList provinces READ GetProvincesQVariantList)

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
	Region(const std::string &identifier);
	virtual ~Region() override;

	virtual void Initialize() override;

	const std::vector<Province *> &GetProvinces() const
	{
		return this->Provinces;
	}

	QVariantList GetProvincesQVariantList() const;

	Q_INVOKABLE void AddProvince(Province *province);
	Q_INVOKABLE void RemoveProvince(Province *province);

	std::vector<Holding *> GetHoldings() const;

	const std::vector<std::unique_ptr<PopulationUnit>> &GetPopulationUnits() const
	{
		return this->PopulationUnits;
	}

	void AddPopulationUnit(std::unique_ptr<PopulationUnit> &&population_unit);

signals:
	void ProvincesChanged();

private:
	std::vector<Province *> Provinces;
	std::vector<std::unique_ptr<PopulationUnit>> PopulationUnits; //population units set for this region in history, used during initialization to generate population units in the region's settlements
};

}
