#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>

class Character;

class LandedTitle : public DataEntry<>, public DataType<LandedTitle>
{
public:
	LandedTitle(const std::string &identifier) : DataEntry(identifier) {}

	static constexpr const char *ClassIdentifier = "landed_title";
	static constexpr const char *DatabaseFolder = "landed_titles";

	const std::string &GetName() const
	{
		return this->Name;
	}

	Character *GetOwner() const
	{
		return this->Owner;
	}

private:
	std::string Name;
	Character *Owner = nullptr;
};
