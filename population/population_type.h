#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QColor>
#include <QVariant>

#include <set>
#include <string>

namespace metternich {

class holding_type;

class population_type final : public data_entry, public data_type<population_type>
{
	Q_OBJECT

	Q_PROPERTY(QColor color MEMBER color READ get_color)
	Q_PROPERTY(QString icon_tag READ get_icon_tag_qstring WRITE set_icon_tag_qstring)
	Q_PROPERTY(QVariantList equivalent_types READ get_equivalent_types_qvariant_list)
	Q_PROPERTY(QVariantList holding_types READ get_holding_types_qvariant_list)
	Q_PROPERTY(bool employable READ is_employable CONSTANT)

public:
	static constexpr const char *class_identifier = "population_type";
	static constexpr const char *database_folder = "population_types";

public:
	population_type(const std::string &identifier) : data_entry(identifier) {}

	virtual void check() const override;

	const QColor &get_color() const
	{
		return this->color;
	}

	const std::string &get_icon_tag() const
	{
		if (this->icon_tag.empty()) {
			return this->get_identifier();
		}

		return this->icon_tag;
	}

	void set_icon_tag(const std::string &icon_tag)
	{
		if (icon_tag == this->get_icon_tag()) {
			return;
		}

		this->icon_tag = icon_tag;
	}

	QString get_icon_tag_qstring() const
	{
		return QString::fromStdString(this->get_icon_tag());
	}

	void set_icon_tag_qstring(const QString &icon_tag)
	{
		this->set_icon_tag(icon_tag.toStdString());
	}

	const std::set<population_type *> &get_equivalent_types() const
	{
		return this->equivalent_types;
	}

	QVariantList get_equivalent_types_qvariant_list() const;

	Q_INVOKABLE void add_equivalent_type(population_type *type)
	{
		this->equivalent_types.insert(type);
	}

	Q_INVOKABLE void remove_equivalent_type(population_type *type)
	{
		this->equivalent_types.erase(type);
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

	bool is_employable() const
	{
		return this->employable;
	}

	void set_employable(const bool employable)
	{
		if (employable == this->is_employable()) {
			return;
		}

		this->employable = employable;
	}

private:
	QColor color;
	std::string icon_tag;
	std::set<population_type *> equivalent_types; //the population types which are equivalent to this one
	std::set<holding_type *> holding_types; //the holding types where this population type can live
	bool employable = false; //whether the population type is employable, i.e. whether any employment type has it designated as an employee
};

}
