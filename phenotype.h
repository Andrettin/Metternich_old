#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>

namespace metternich {

class phenotype : public DataEntry, public DataType<phenotype>
{
	Q_OBJECT

public:
	phenotype(const std::string &identifier) : DataEntry(identifier) {}

	static constexpr const char *ClassIdentifier = "phenotype";
	static constexpr const char *DatabaseFolder = "phenotypes";
};

}
