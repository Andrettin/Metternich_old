#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>

namespace Metternich {

class Dynasty;

class Culture : public DataEntry, public DataType<Culture>
{
	Q_OBJECT

public:
	static constexpr const char *ClassIdentifier = "culture";
	static constexpr const char *DatabaseFolder = "cultures";

	Culture(const std::string &identifier) : DataEntry(identifier) {}

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

	void AddDynasty(Dynasty *dynasty)
	{
		this->Dynasties.push_back(dynasty);
	}

	std::string GenerateMaleName() const;
	std::string GenerateFemaleName() const;
	std::string GenerateDynastyName() const;

private:
	std::vector<std::string> MaleNames;
	std::vector<std::string> FemaleNames;
	std::vector<Dynasty *> Dynasties;
};

}
