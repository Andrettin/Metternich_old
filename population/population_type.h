#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QColor>
#include <QVariant>

#include <set>
#include <string>

namespace Metternich {

class HoldingType;

class PopulationType : public DataEntry, public DataType<PopulationType>
{
	Q_OBJECT

	Q_PROPERTY(QColor color MEMBER Color READ GetColor)
	Q_PROPERTY(QVariantList holding_types READ GetHoldingTypesQVariantList)

public:
	static constexpr const char *ClassIdentifier = "population_type";
	static constexpr const char *DatabaseFolder = "population_types";

public:
	PopulationType(const std::string &identifier) : DataEntry(identifier) {}

	virtual void ProcessGSMLScope(const gsml_data &scope) override;
	virtual void Check() const override;

	const QColor &GetColor() const
	{
		return this->Color;
	}

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
	QColor Color;
	std::set<HoldingType *> HoldingTypes; //the holding types where this population type can live
};

}
