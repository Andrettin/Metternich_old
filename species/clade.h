#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <set>

namespace metternich {

class species;

class clade : public data_entry, public data_type<clade>
{
	Q_OBJECT

public:
	static constexpr const char *class_identifier = "clade";
	static constexpr const char *database_folder = "clades";

	clade(const std::string &identifier) : data_entry(identifier) {}

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

private:
	std::set<species *> species;
};

}
