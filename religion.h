#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>

namespace Metternich {

class Religion : public DataEntry, public DataType<Religion>
{
	Q_OBJECT

public:
	Religion(const std::string &identifier) : DataEntry(identifier) {}

	static constexpr const char *ClassIdentifier = "religion";
	static constexpr const char *DatabaseFolder = "religions";
};

}
