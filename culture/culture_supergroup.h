#pragma once

#include "culture/culture_base.h"
#include "database/data_type.h"

namespace metternich {

class culture_supergroup final : public culture_base, public data_type<culture_supergroup>
{
	Q_OBJECT

public:
	static constexpr const char *class_identifier = "culture_supergroup";
	static constexpr const char *database_folder = "culture_supergroups";

	culture_supergroup(const std::string &identifier) : culture_base(identifier) {}
};

}
