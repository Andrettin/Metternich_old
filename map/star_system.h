#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QGeoCoordinate>
#include <QPointF>

namespace metternich {

class world;

class star_system : public data_entry, public data_type<star_system>
{
	Q_OBJECT

	Q_PROPERTY(QGeoCoordinate astrocoordinate READ get_astrocoordinate CONSTANT)
	Q_PROPERTY(int astrodistance READ get_astrodistance CONSTANT)
	Q_PROPERTY(bool binary MEMBER binary READ is_binary)
	Q_PROPERTY(QVariantList worlds READ get_worlds_qvariant_list CONSTANT)

public:
	static constexpr const char *class_identifier = "star_system";
	static constexpr const char *database_folder = "star_systems";
	static constexpr int million_km_per_pixel = 1;
	static constexpr int min_orbit_distance = 32; //minimum distance between an orbit and the next one in the system
	static constexpr int max_orbit_distance = 128; //maximum distance between an orbit and the next one in the system

public:
	star_system(const std::string &identifier) : data_entry(identifier)
	{
	}

	virtual void initialize() override;

	const QGeoCoordinate &get_astrocoordinate() const
	{
		return this->astrocoordinate;
	}

	int get_astrodistance() const
	{
		return this->astrodistance;
	}

	bool is_binary() const
	{
		return this->binary;
	}

	const std::vector<world *> &get_worlds() const
	{
		return this->worlds;
	}

	QVariantList get_worlds_qvariant_list() const;

	void add_world(world *world)
	{
		this->worlds.push_back(world);
	}

	void remove_world(world *world);

private:
	QGeoCoordinate astrocoordinate;
	int astrodistance = 0;
	bool binary = false; //whether this is a binary star system
	std::vector<world *> worlds;
};

}
