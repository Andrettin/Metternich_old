#pragma once

#include <QObject>

namespace metternich {

class building;
class holding;

/**
**	@brief	The slot for a given building in a holding
*/
class building_slot : public QObject
{
	Q_OBJECT

public:
	building_slot(building *building, holding *holding) : building(building), holding(holding)
	{
	}

	~building_slot();

	building *get_building() const
	{
		return this->building;
	}

	bool is_available() const
	{
		return this->available;
	}

	bool is_buildable() const
	{
		return this->buildable;
	}

	bool is_built() const
	{
		return this->built;
	}

	void set_built(const bool built);

signals:
	void built_changed();

private:
	building *building = nullptr;
	holding *holding = nullptr; //the building slot's holding
	bool available = false; //whether building is at all available
	bool buildable = false; //whether the building is buildable
	bool built = false;
};

}
