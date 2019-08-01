#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>

namespace Metternich {

class Culture;

class Dynasty : public DataEntry, public DataType<Dynasty>
{
	Q_OBJECT

	Q_PROPERTY(Metternich::Culture* culture READ GetCulture WRITE SetCulture)

public:
	Dynasty(const std::string &identifier) : DataEntry(identifier) {}

	static constexpr const char *ClassIdentifier = "dynasty";
	static constexpr const char *DatabaseFolder = "dynasties";

	Culture *GetCulture() const
	{
		return this->Culture;
	}

	void SetCulture(Culture *culture);

private:
	Culture *Culture = nullptr;
};

}
