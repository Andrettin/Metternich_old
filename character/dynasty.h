#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>

namespace metternich {

class Culture;

class Dynasty : public DataEntry, public DataType<Dynasty>
{
	Q_OBJECT

	Q_PROPERTY(metternich::Culture* culture READ GetCulture WRITE SetCulture)

public:
	Dynasty(const std::string &identifier) : DataEntry(identifier) {}

	static constexpr const char *ClassIdentifier = "dynasty";
	static constexpr const char *DatabaseFolder = "dynasties";

	metternich::Culture *GetCulture() const
	{
		return this->Culture;
	}

	void SetCulture(Culture *culture);

private:
	metternich::Culture *Culture = nullptr;
};

}
