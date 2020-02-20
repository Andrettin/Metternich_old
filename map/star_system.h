#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QPolygonF>

namespace metternich {

class landed_title;
class world;
enum class map_edge;

class star_system : public data_entry, public data_type<star_system>
{
	Q_OBJECT

	Q_PROPERTY(metternich::landed_title* duchy READ get_duchy WRITE set_duchy NOTIFY duchy_changed)
	Q_PROPERTY(metternich::world* primary_star READ get_primary_star CONSTANT)
	Q_PROPERTY(QColor color READ get_color NOTIFY color_changed)
	Q_PROPERTY(QVariantList worlds READ get_worlds_qvariant_list CONSTANT)
	Q_PROPERTY(metternich::map_edge map_edge MEMBER map_edge READ get_map_edge)
	Q_PROPERTY(QVariantList territory_polygon READ get_territory_polygon_qvariant_list CONSTANT)
	Q_PROPERTY(QRectF territory_bounding_rect READ get_territory_bounding_rect CONSTANT)
	Q_PROPERTY(bool ethereal MEMBER ethereal READ is_ethereal NOTIFY ethereal_changed)

public:
	static constexpr const char *class_identifier = "star_system";
	static constexpr const char *database_folder = "star_systems";
	static const inline QColor empty_system_color = QColor("#f5f5dc");

	star_system(const std::string &identifier);

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;

	virtual std::string get_name() const override;

	landed_title *get_duchy() const
	{
		return this->duchy;
	}

	void set_duchy(landed_title *duchy);

	world *get_primary_star() const
	{
		return this->primary_star;
	}

	void calculate_primary_star();

	const QColor &get_color() const;

	QVariantList get_territory_polygon_qvariant_list() const;

	QRectF get_territory_bounding_rect() const
	{
		return this->territory_polygon.boundingRect();
	}

	void calculate_territory_polygon();

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

	map_edge get_map_edge() const
	{
		return this->map_edge;
	}

	bool is_ethereal() const
	{
		return this->ethereal;
	}

signals:
	void duchy_changed();
	void color_changed();
	void ethereal_changed();

private:
	landed_title *duchy = nullptr; //the star system's corresponding cosmic duchy
	world *primary_star = nullptr;
	std::vector<world *> worlds;
	map_edge map_edge;
	std::vector<star_system *> adjacent_systems;
	QPolygonF territory_polygon;
	bool ethereal = false; //whether the star system is an ethereal one (i.e. Asgard's system); ethereal systems can only be entered by ethereal beings, who cannot enter non-ethereal systems, either; these impediments could be surpassed by technological or magical developments
};

}
