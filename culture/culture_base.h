#pragma once

#include "database/data_entry.h"

#include <string>
#include <vector>

namespace metternich {

class phenotype;

/**
**	@brief	The base class for characteristics common to cultures and culture groups
*/
class culture_base : public DataEntry
{
	Q_OBJECT

	Q_PROPERTY(metternich::phenotype* default_phenotype MEMBER default_phenotype)

public:
	culture_base(const std::string &identifier) : DataEntry(identifier) {}

	virtual void ProcessGSMLScope(const gsml_data &scope) override;

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
	phenotype *default_phenotype = nullptr;
	std::vector<std::string> male_names;
	std::vector<std::string> female_names;
};

}
