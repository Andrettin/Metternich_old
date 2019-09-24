#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>

namespace metternich {

class Trait : public data_entry, public DataType<Trait>
{
	Q_OBJECT

public:
	Trait(const std::string &identifier) : data_entry(identifier) {}

	static constexpr const char *ClassIdentifier = "trait";
	static constexpr const char *DatabaseFolder = "traits";
};

}
