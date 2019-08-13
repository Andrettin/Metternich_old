#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QVariant>

#include <set>
#include <string>

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

	const std::set<HoldingType *> &GetHoldingTypes() const
	{
		return this->HoldingTypes;
	}

	QVariantList GetHoldingTypesQVariantList() const;

	Q_INVOKABLE void AddHoldingType(HoldingType *holding_type)
	{
		this->HoldingTypes.insert(holding_type);
	}

	Q_INVOKABLE void RemoveHoldingType(HoldingType *holding_type)
	{
		this->HoldingTypes.erase(holding_type);
	}

private:
	std::set<HoldingType *> HoldingTypes; //the holding types where this population type can live
};

}
