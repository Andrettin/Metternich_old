#pragma once

#include <set>
#include <string>

class Building;
class HoldingType;

class Holding
{
public:
	static constexpr const char *ClassIdentifier = "holding";

	const std::string &GetName() const
	{
		return this->Name;
	}

	const HoldingType *GetType() const
	{
		return this->Type;
	}

	const std::set<Building *> &GetBuildings() const
	{
		return this->Buildings;
	}

private:
	std::string Name;
	const HoldingType *Type = nullptr;
	std::set<Building *> Buildings;
};
