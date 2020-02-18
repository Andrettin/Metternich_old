#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QPolygonF>

namespace metternich {

class world;

class star_system : public data_entry, public data_type<star_system>
{
	Q_OBJECT

	Q_PROPERTY(metternich::world* primary_star READ get_primary_star CONSTANT)
	Q_PROPERTY(QVariantList territory_polygon READ get_territory_polygon_qvariant_list CONSTANT)
	Q_PROPERTY(QRectF territory_bounding_rect READ get_territory_bounding_rect CONSTANT)
	Q_PROPERTY(QVariantList worlds READ get_worlds_qvariant_list CONSTANT)

public:
	static constexpr const char *class_identifier = "star_system";
	static constexpr const char *database_folder = "star_systems";

	star_system(const std::string &identifier) : data_entry(identifier)
	{
	}

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;

	world *get_primary_star() const
	{
		return this->primary_star;
	}

	void calculate_primary_star();

	QVariantList get_territory_polygon_qvariant_list() const;

	QRectF get_territory_bounding_rect() const
	{
		return this->territory_polygon.boundingRect();
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
	world *primary_star = nullptr;
	std::vector<world *> worlds;
	std::vector<star_system *> adjacent_systems;
	QPolygonF territory_polygon;
};

}
