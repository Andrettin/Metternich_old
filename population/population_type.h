#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QColor>
#include <QVariant>

#include <set>
#include <string>

namespace metternich {

class holding_type;

class population_type : public data_entry, public data_type<population_type>
{
	Q_OBJECT

	Q_PROPERTY(QColor color MEMBER color READ get_color)
	Q_PROPERTY(QVariantList holding_types READ get_holding_types_qvariant_list)

public:
	static constexpr const char *class_identifier = "population_type";
	static constexpr const char *database_folder = "population_types";

public:
	population_type(const std::string &identifier) : data_entry(identifier) {}

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	const QColor &get_color() const
	{
		return this->color;
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
	QColor color;
	std::set<holding_type *> holding_types; //the holding types where this population type can live
};

}
