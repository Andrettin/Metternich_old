#pragma once

#include "culture/culture_base.h"
#include "database/data_type.h"

namespace Metternich {

class CultureGroup : public CultureBase, public DataType<CultureGroup>
{
	Q_OBJECT

public:
	static constexpr const char *ClassIdentifier = "culture_group";
	static constexpr const char *DatabaseFolder = "culture_groups";

	CultureGroup(const std::string &identifier) : CultureBase(identifier) {}
};

}
