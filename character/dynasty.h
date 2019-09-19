#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>

namespace metternich {

class culture;

class Dynasty : public DataEntry, public DataType<Dynasty>
{
	Q_OBJECT

	Q_PROPERTY(metternich::culture* culture READ get_culture WRITE set_culture)

public:
	Dynasty(const std::string &identifier) : DataEntry(identifier) {}

	static constexpr const char *ClassIdentifier = "dynasty";
	static constexpr const char *DatabaseFolder = "dynasties";

	metternich::culture *get_culture() const
	{
		return this->culture;
	}

	void set_culture(culture *culture);

private:
	metternich::culture *culture = nullptr;
};

}
