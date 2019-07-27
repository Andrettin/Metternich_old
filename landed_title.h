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
	static constexpr const char *BaronyPrefix = "b_";
	static constexpr const char *CountyPrefix = "c_";
	static constexpr const char *DuchyPrefix = "d_";
	static constexpr const char *KingdomPrefix = "k_";
	static constexpr const char *EmpirePrefix = "e_";

	static LandedTitle *Add(const std::string &identifier);

	Character *GetOwner() const
	{
		return this->Owner;
	}

private:
	Character *Owner = nullptr;
};
