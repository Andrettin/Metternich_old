#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QColor>
#include <QVariant>

#include <set>
#include <string>

namespace metternich {

class holding_type;

class PopulationType : public data_entry, public DataType<PopulationType>
{
	Q_OBJECT

	Q_PROPERTY(QColor color MEMBER Color READ GetColor)
	Q_PROPERTY(QVariantList holding_types READ get_holding_types_qvariant_list)

public:
	static constexpr const char *ClassIdentifier = "population_type";
	static constexpr const char *DatabaseFolder = "population_types";

public:
	PopulationType(const std::string &identifier) : data_entry(identifier) {}

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	const QColor &GetColor() const
	{
		return this->Color;
	}

	const std::set<holding_type *> &get_holding_types() const
	{
		return this->holding_types;
	}

	QVariantList get_holding_types_qvariant_list() const;

	Q_INVOKABLE void add_holding_type(holding_type *holding_type)
	{
		this->holding_types.insert(holding_type);
	}

	Q_INVOKABLE void remove_holding_type(holding_type *holding_type)
	{
		this->holding_types.erase(holding_type);
	}

private:
	QColor Color;
	std::set<holding_type *> holding_types; //the holding types where this population type can live
};

}
