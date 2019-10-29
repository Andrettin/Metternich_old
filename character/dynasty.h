#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>

namespace metternich {

class culture;

class dynasty : public data_entry, public data_type<dynasty>
{
	Q_OBJECT

	Q_PROPERTY(metternich::culture* culture READ get_culture WRITE set_culture)

public:
	dynasty(const std::string &identifier) : data_entry(identifier) {}

	static constexpr const char *class_identifier = "dynasty";
	static constexpr const char *database_folder = "dynasties";

	metternich::culture *get_culture() const
	{
		return this->culture;
	}

	void set_culture(culture *culture);

private:
	metternich::culture *culture = nullptr;
};

}
