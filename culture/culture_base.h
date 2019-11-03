#pragma once

#include "database/data_entry.h"

#include <QColor>

#include <string>
#include <vector>

namespace metternich {

class phenotype;

/**
**	@brief	The base class for characteristics common to cultures and culture groups
*/
class culture_base : public data_entry
{
	Q_OBJECT

	Q_PROPERTY(QColor color MEMBER color READ get_color)
	Q_PROPERTY(metternich::phenotype* default_phenotype MEMBER default_phenotype)

public:
	culture_base(const std::string &identifier) : data_entry(identifier) {}

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	const QColor &get_color() const
	{
		return this->color;
	}

	phenotype *get_default_phenotype() const
	{
		return this->default_phenotype;
	}

	const std::vector<std::string> &get_male_names() const
	{
		return this->male_names;
	}

	const std::vector<std::string> &get_female_names() const
	{
		return this->female_names;
	}

private:
	QColor color;
	phenotype *default_phenotype = nullptr;
	std::vector<std::string> male_names;
	std::vector<std::string> female_names;
};

}
