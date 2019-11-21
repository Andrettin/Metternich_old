#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <map>
#include <string>

namespace metternich {

class species : public data_entry, public data_type<species>
{
	Q_OBJECT

	Q_PROPERTY(bool sapient MEMBER sapient READ is_sapient)

public:
	static constexpr const char *class_identifier = "species";
	static constexpr const char *database_folder = "species";

	species(const std::string &identifier) : data_entry(identifier) {}

	bool is_sapient() const
	{
		return this->sapient;
	}

private:
	bool sapient = false; //whether the species is sapient
};

}
