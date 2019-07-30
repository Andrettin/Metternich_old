#pragma once

#include <string>
#include <vector>

namespace Metternich {

class Building;

class HoldingType
{
public:
	static constexpr const char *ClassIdentifier = "holding_type";

	const std::string &GetName() const
	{
		return this->Name;
	}

	const std::vector<Building *> &GetBuildings() const
	{
		return this->Buildings;
	}

private:
	std::string Name;
	std::vector<Building *> Buildings;
};

}
