#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>

namespace metternich {

class law_group;

class law final : public data_entry, public data_type<law>
{
	Q_OBJECT

	Q_PROPERTY(metternich::law_group* group MEMBER group READ get_group)

public:
	law(const std::string &identifier) : data_entry(identifier) {}

	static constexpr const char *class_identifier = "law";
	static constexpr const char *database_folder = "laws";

	metternich::law_group *get_group() const
	{
		return this->group;
	}

private:
	metternich::law_group *group = nullptr;
};

}
