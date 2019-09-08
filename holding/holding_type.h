#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>
#include <vector>

namespace metternich {

class Building;
class Modifier;

class holding_type : public DataEntry, public DataType<holding_type>
{
	Q_OBJECT

	Q_PROPERTY(bool settlement MEMBER settlement READ is_settlement)

public:
	static constexpr const char *ClassIdentifier = "holding_type";
	static constexpr const char *DatabaseFolder = "holding_types";

	holding_type(const std::string &identifier);
	virtual ~holding_type() override;

	virtual void ProcessGSMLScope(const gsml_data &scope) override;

	bool is_settlement() const
	{
		return this->settlement;
	}

	const std::vector<Building *> &get_buildings() const
	{
		return this->buildings;
	}

	void add_building(Building *building)
	{
		this->buildings.push_back(building);
	}

	void remove_building(Building *building)
	{
		this->buildings.erase(std::remove(this->buildings.begin(), this->buildings.end(), building), this->buildings.end());
	}

	const std::unique_ptr<metternich::Modifier> &get_modifier() const
	{
		return this->modifier;
	}

private:
	bool settlement = false;	//whether the holding type occupies a settlement slot
	std::vector<Building *> buildings;
	std::unique_ptr<metternich::Modifier> modifier; //the modifier applied to holdings of this type
};

}
