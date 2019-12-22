#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QColor>

#include <set>

namespace metternich {

class province;
class species;

class clade : public data_entry, public data_type<clade>
{
	Q_OBJECT

	Q_PROPERTY(QColor color MEMBER color READ get_color)
	Q_PROPERTY(QVariantList provinces READ get_provinces_qvariant_list NOTIFY provinces_changed)

public:
	static constexpr const char *class_identifier = "clade";
	static constexpr const char *database_folder = "clades";

	clade(const std::string &identifier) : data_entry(identifier) {}

	virtual void check() const override
	{
		if (!this->get_color().isValid()) {
			throw std::runtime_error("Clade \"" + this->get_identifier() + "\" has no valid color.");
		}
	}

	const QColor &get_color() const
	{
		return this->color;
	}

	const std::set<species *> &get_species() const
	{
		return this->species;
	}

	void add_species(species *species)
	{
		this->species.insert(species);
	}

	void remove_species(species *species)
	{
		this->species.erase(species);
	}

	const std::set<province *> &get_provinces() const
	{
		return this->provinces;
	}

	QVariantList get_provinces_qvariant_list() const;

	void add_province(province *province)
	{
		this->provinces.insert(province);
		emit provinces_changed();
	}

	void remove_province(province *province)
	{
		this->provinces.erase(province);
		emit provinces_changed();
	}

	bool is_alive() const
	{
		return !this->get_provinces().empty();
	}

	bool is_ai() const;

signals:
	void provinces_changed();

private:
	QColor color;
	std::set<species *> species;
	std::set<province *> provinces; //provinces owned by the clade
};

}
