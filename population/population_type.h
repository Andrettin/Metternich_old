#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QVariant>

#include <string>
#include <vector>

namespace Metternich {

class HoldingType;

class PopulationType : public DataEntry, public DataType<PopulationType>
{
	Q_OBJECT

	Q_PROPERTY(QVariantList holding_types READ GetHoldingTypesQVariantList)
	Q_PROPERTY(bool default_type READ IsDefaultType WRITE SetDefaultType NOTIFY DefaultTypeChanged)

public:
	static constexpr const char *ClassIdentifier = "population_type";
	static constexpr const char *DatabaseFolder = "population_types";

	static PopulationType *GetDefaultType()
	{
		return PopulationType::DefaultType;
	}

private:
	static inline PopulationType *DefaultType = nullptr;

public:
	PopulationType(const std::string &identifier) : DataEntry(identifier) {}

	const std::vector<HoldingType *> &GetHoldingTypes() const
	{
		return this->HoldingTypes;
	}

	QVariantList GetHoldingTypesQVariantList() const;

	Q_INVOKABLE void AddHoldingType(HoldingType *holding_type)
	{
		this->HoldingTypes.push_back(holding_type);
	}

	Q_INVOKABLE void RemoveHoldingType(HoldingType *holding_type)
	{
		this->HoldingTypes.erase(std::remove(this->HoldingTypes.begin(), this->HoldingTypes.end(), holding_type), this->HoldingTypes.end());
	}

	bool IsDefaultType() const
	{
		return PopulationType::GetDefaultType() == this;
	}

	void SetDefaultType(const bool default_type)
	{
		if (default_type == (PopulationType::GetDefaultType() == this)) {
			return;
		}

		if (default_type) {
			if (PopulationType::GetDefaultType() != nullptr) {
				PopulationType::GetDefaultType()->SetDefaultType(false);
			}

			PopulationType::DefaultType = this;

			emit DefaultTypeChanged();
		} else {
			PopulationType::DefaultType = nullptr;

			emit DefaultTypeChanged();
		}
	}

signals:
	void DefaultTypeChanged();

private:
	std::vector<HoldingType *> HoldingTypes; //the holding types where this population type can live
};

}
