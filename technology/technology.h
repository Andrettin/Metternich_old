#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

namespace metternich {

class technology_category;

class technology : public data_entry, public data_type<technology>
{
	Q_OBJECT

	Q_PROPERTY(metternich::technology_category* category MEMBER category READ get_category)

public:
	technology(const std::string &identifier) : data_entry(identifier) {}

	static constexpr const char *class_identifier = "technology";
	static constexpr const char *database_folder = "technologies";

	technology_category *get_category() const
	{
		return this->category;
	}

private:
	technology_category *category = nullptr;
};

}
