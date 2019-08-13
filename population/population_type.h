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

public:
	static constexpr const char *ClassIdentifier = "population_type";
	static constexpr const char *DatabaseFolder = "population_types";

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

private:
	std::vector<HoldingType *> HoldingTypes; //the holding types where this population type can live
};

}
