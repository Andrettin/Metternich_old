#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QColor>

namespace Metternich {

class Terrain : public DataEntry, public DataType<Terrain>
{
	Q_OBJECT

	Q_PROPERTY(QColor color READ GetColor CONSTANT)

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

private:
	QColor Color; //the color used to identify the terrain in the terrain map
};

}
