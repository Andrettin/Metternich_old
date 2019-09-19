#pragma once

#include "culture/culture_base.h"
#include "database/data_type.h"

namespace metternich {

class culture_group : public culture_base, public DataType<culture_group>
{
	Q_OBJECT

public:
	static constexpr const char *ClassIdentifier = "culture_group";
	static constexpr const char *DatabaseFolder = "culture_groups";

	culture_group(const std::string &identifier) : culture_base(identifier) {}
};

}
