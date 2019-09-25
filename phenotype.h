#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>

namespace metternich {

class phenotype : public data_entry, public data_type<phenotype>
{
	Q_OBJECT

public:
	phenotype(const std::string &identifier) : data_entry(identifier) {}

	static constexpr const char *class_identifier = "phenotype";
	static constexpr const char *database_folder = "phenotypes";
};

}
