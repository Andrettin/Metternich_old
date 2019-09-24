#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>

namespace metternich {

class law_group;

class law : public data_entry, public DataType<law>
{
	Q_OBJECT

	Q_PROPERTY(metternich::law_group* group MEMBER group READ get_group)

public:
	law(const std::string &identifier) : data_entry(identifier) {}

	static constexpr const char *ClassIdentifier = "law";
	static constexpr const char *DatabaseFolder = "laws";

	metternich::law_group *get_group() const
	{
		return this->group;
	}

private:
	metternich::law_group *group = nullptr;
};

}
