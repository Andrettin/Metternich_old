#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>

namespace Metternich {

class Culture : public DataEntry, public DataType<Culture>
{
	Q_OBJECT

public:
	Culture(const std::string &identifier) : DataEntry(identifier) {}

	static constexpr const char *ClassIdentifier = "culture";
	static constexpr const char *DatabaseFolder = "cultures";
};

}
