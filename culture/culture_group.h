#pragma once

#include "culture/culture_base.h"
#include "database/data_type.h"

namespace metternich {

class culture_group : public culture_base, public data_type<culture_group>
{
	Q_OBJECT

public:
	static constexpr const char *class_identifier = "culture_group";
	static constexpr const char *database_folder = "culture_groups";

	culture_group(const std::string &identifier) : culture_base(identifier) {}
};

}
