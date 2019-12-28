#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>

namespace metternich {

class technology_category : public data_entry, public data_type<technology_category>
{
	Q_OBJECT

public:
	technology_category(const std::string &identifier) : data_entry(identifier) {}

	static constexpr const char *class_identifier = "technology_category";
	static constexpr const char *database_folder = "technology_categories";
};

}
