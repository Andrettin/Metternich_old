#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>

namespace Metternich {

class Dynasty;

class CultureGroup : public DataEntry, public DataType<CultureGroup>
{
	Q_OBJECT

public:
	static constexpr const char *ClassIdentifier = "culture_group";
	static constexpr const char *DatabaseFolder = "culture_groups";

	CultureGroup(const std::string &identifier) : DataEntry(identifier) {}
};

}
