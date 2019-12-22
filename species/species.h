#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <set>

namespace metternich {

class clade;

class species : public data_entry, public data_type<species>
{
	Q_OBJECT

	Q_PROPERTY(bool sapient MEMBER sapient READ is_sapient)
	Q_PROPERTY(QVariantList evolutions READ get_evolutions_qvariant_list)
	Q_PROPERTY(metternich::clade* clade READ get_clade WRITE set_clade)

public:
	static constexpr const char *class_identifier = "species";
	static constexpr const char *database_folder = "species";

	species(const std::string &identifier) : data_entry(identifier) {}

	bool is_sapient() const
	{
		return this->sapient;
	}

	const std::set<species *> &get_evolutions() const
	{
		return this->evolutions;
	}

	QVariantList get_evolutions_qvariant_list() const;

	Q_INVOKABLE void add_evolution(species *evolution)
	{
		this->evolutions.insert(evolution);
	}

	Q_INVOKABLE void remove_evolution(species *evolution)
	{
		this->evolutions.erase(evolution);
	}

	clade *get_clade() const
	{
		return this->clade;
	}

	void set_clade(clade *clade);

private:
	bool sapient = false; //whether the species is sapient
	std::set<species *> evolutions;
	clade *clade = nullptr;
};

}
