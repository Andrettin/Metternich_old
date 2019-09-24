#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>

namespace metternich {

class phenotype : public data_entry, public DataType<phenotype>
{
	Q_OBJECT

public:
	phenotype(const std::string &identifier) : data_entry(identifier) {}

	static constexpr const char *ClassIdentifier = "phenotype";
	static constexpr const char *DatabaseFolder = "phenotypes";
};

}
