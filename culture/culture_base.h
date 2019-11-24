#pragma once

#include "database/data_entry.h"

#include <QColor>

#include <set>
#include <string>

namespace metternich {

class phenotype;
class species;

/**
**	@brief	The base class for characteristics common to cultures and culture groups
*/
class culture_base : public data_entry
{
	Q_OBJECT

	Q_PROPERTY(metternich::species* species MEMBER species READ get_species)
	Q_PROPERTY(QColor color MEMBER color READ get_color)
	Q_PROPERTY(metternich::phenotype* default_phenotype MEMBER default_phenotype READ get_default_phenotype)

public:
	culture_base(const std::string &identifier) : data_entry(identifier) {}

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	species *get_species() const
	{
		return this->species;
	}

	const QColor &get_color() const
	{
		return this->color;
	}

	phenotype *get_default_phenotype() const
	{
		return this->default_phenotype;
	}

	const std::set<std::string> &get_male_names() const
	{
		return this->male_names;
	}

	bool has_male_name(const std::string &name) const
	{
		return this->male_names.contains(name);
	}

	const std::set<std::string> &get_female_names() const
	{
		return this->female_names;
	}

	bool has_female_name(const std::string &name) const
	{
		return this->female_names.contains(name);
	}

protected:
	void set_species(species *species)
	{
		this->species = species;
	}

	void set_default_phenotype(phenotype *phenotype)
	{
		this->default_phenotype = phenotype;
	}

	void add_male_name(const std::string &name)
	{
		this->male_names.insert(name);
	}

	void add_female_name(const std::string &name)
	{
		this->female_names.insert(name);
	}

private:
	species *species = nullptr;
	QColor color;
	phenotype *default_phenotype = nullptr;
	std::set<std::string> male_names;
	std::set<std::string> female_names;
};

}
