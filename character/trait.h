#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>

namespace metternich {

class Trait : public DataEntry, public DataType<Trait>
{
	Q_OBJECT

public:
	Trait(const std::string &identifier) : DataEntry(identifier) {}

	static constexpr const char *ClassIdentifier = "trait";
	static constexpr const char *DatabaseFolder = "traits";
};

}
