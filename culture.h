#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>

namespace Metternich {

class Culture : public DataEntry, public DataType<Culture>
{
	Q_OBJECT

public:
	Culture(const std::string &identifier) : DataEntry(identifier) {}

	static constexpr const char *ClassIdentifier = "culture";
	static constexpr const char *DatabaseFolder = "cultures";

	virtual void ProcessGSMLScope(const GSMLData &scope) override;

	virtual void Check() const override
	{
		if (this->MaleNames.empty()) {
			throw std::runtime_error("Culture \"" + this->GetIdentifier() + "\" has no male names.");
		}

		if (this->FemaleNames.empty()) {
			throw std::runtime_error("Culture \"" + this->GetIdentifier() + "\" has no female names.");
		}
	}

	std::string GenerateMaleName() const;
	std::string GenerateFemaleName() const;

private:
	std::vector<std::string> MaleNames;
	std::vector<std::string> FemaleNames;
};

}
