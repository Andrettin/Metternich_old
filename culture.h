#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <random>
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

	std::string GenerateMaleName() const
	{
		if (this->MaleNames.empty()) {
			return std::string();
		}

		std::random_device random_device;
		std::uniform_int_distribution<size_t> distribution(0, this->MaleNames.size() - 1);

		return this->MaleNames[distribution(random_device)];
	}

	std::string GenerateFemaleName() const
	{
		if (this->FemaleNames.empty()) {
			return std::string();
		}

		std::random_device random_device;
		std::uniform_int_distribution<size_t> distribution(0, this->FemaleNames.size() - 1);

		return this->FemaleNames[distribution(random_device)];
	}

private:
	std::vector<std::string> MaleNames;
	std::vector<std::string> FemaleNames;
};

}
