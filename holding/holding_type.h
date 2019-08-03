#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>
#include <vector>

namespace Metternich {

class Building;

class HoldingType : public DataEntry, public DataType<HoldingType>
{
	Q_OBJECT

	Q_PROPERTY(bool settlement MEMBER Settlement READ IsSettlement)

public:
	static constexpr const char *ClassIdentifier = "holding_type";
	static constexpr const char *DatabaseFolder = "holding_types";

	HoldingType(const std::string &identifier) : DataEntry(identifier) {}

	bool IsSettlement() const
	{
		return this->Settlement;
	}

	const std::vector<Building *> &GetBuildings() const
	{
		return this->Buildings;
	}

private:
	bool Settlement = false;	//whether the holding type occupies a settlement slot
	std::vector<Building *> Buildings;
};

}
