#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>

namespace metternich {

class law_group : public data_entry, public DataType<law_group>
{
	Q_OBJECT

public:
	law_group(const std::string &identifier) : data_entry(identifier) {}

	static constexpr const char *ClassIdentifier = "law_group";
	static constexpr const char *DatabaseFolder = "law_groups";
};

}
