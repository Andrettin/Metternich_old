#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QVariant>

#include <memory>
#include <vector>

namespace metternich {

class holding;
class population_unit;
class Province;

class Region : public DataEntry, public DataType<Region>
{
	Q_OBJECT

	Q_PROPERTY(QVariantList provinces READ GetProvincesQVariantList)
	Q_PROPERTY(QVariantList subregions READ GetSubregionsQVariantList)

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

	virtual void Initialize() override
	{
		//add each subregion's provinces to this one
		for (Region *subregion : this->Subregions) {
			if (!subregion->IsInitialized()) {
				subregion->Initialize();
			}

			for (Province *province : subregion->GetProvinces()) {
				this->AddProvince(province);
			}
		}

		DataEntryBase::Initialize();
	}

	virtual void initialize_history() override;

	const std::vector<Province *> &GetProvinces() const
	{
		return this->Provinces;
	}

	QVariantList GetProvincesQVariantList() const;
	Q_INVOKABLE void AddProvince(Province *province);
	Q_INVOKABLE void RemoveProvince(Province *province);

	QVariantList GetSubregionsQVariantList() const;

	Q_INVOKABLE void AddSubregion(Region *subregion)
	{
		this->Subregions.push_back(subregion);
	}

	Q_INVOKABLE void RemoveSubregion(Region *subregion)
	{
		this->Subregions.erase(std::remove(this->Subregions.begin(), this->Subregions.end(), subregion), this->Subregions.end());
	}

	std::vector<holding *> get_holdings() const;

	const std::vector<std::unique_ptr<population_unit>> &get_population_units() const
	{
		return this->population_units;
	}

	void add_population_unit(std::unique_ptr<population_unit> &&population_unit);

signals:
	void ProvincesChanged();

private:
	std::vector<Province *> Provinces;
	std::vector<Region *> Subregions; //subregions of this region
	std::vector<std::unique_ptr<population_unit>> population_units; //population units set for this region in history, used during initialization to generate population units in the region's settlements
};

}
