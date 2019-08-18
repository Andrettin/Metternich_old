#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QColor>

namespace Metternich {

class Terrain : public DataEntry, public DataType<Terrain>
{
	Q_OBJECT

	Q_PROPERTY(QColor color READ GetColor CONSTANT)
	Q_PROPERTY(bool water MEMBER Water READ IsWater)
	Q_PROPERTY(bool navigable MEMBER Navigable READ IsNavigable)
	Q_PROPERTY(int life_rating MEMBER LifeRating READ GetLifeRating)

public:
	static constexpr const char *ClassIdentifier = "terrain";
	static constexpr const char *DatabaseFolder = "terrains";

	static Terrain *GetByRGB(const QRgb &rgb, const bool should_find = true);

private:
	static inline std::map<QRgb, Terrain *> InstancesByRGB;

public:
	Terrain(const std::string &identifier) : DataEntry(identifier) {}

	virtual void ProcessGSMLScope(const GSMLData &scope) override;

	const QColor &GetColor() const
	{
		return this->Color;
	}

	bool IsWater() const
	{
		return this->Water;
	}

	bool IsNavigable() const
	{
		return this->Navigable;
	}

	int GetLifeRating() const
	{
		return this->LifeRating;
	}

private:
	QColor Color; //the color used to identify the terrain in the terrain map
	bool Water = false; //whether the terrain is a water terrain
	bool Navigable = false; //whether this water terrain is navigable
	int LifeRating = 0; //the life rating of the terrain
};

}
