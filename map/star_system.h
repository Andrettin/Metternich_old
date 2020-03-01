#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QPolygonF>

namespace metternich {

class landed_title;
class world;
enum class map_mode;

class star_system : public data_entry, public data_type<star_system>
{
	Q_OBJECT

	Q_PROPERTY(metternich::landed_title* duchy READ get_duchy WRITE set_duchy NOTIFY duchy_changed)
	Q_PROPERTY(metternich::landed_title* kingdom READ get_kingdom NOTIFY kingdom_changed)
	Q_PROPERTY(metternich::landed_title* de_jure_kingdom READ get_de_jure_kingdom NOTIFY de_jure_kingdom_changed)
	Q_PROPERTY(metternich::landed_title* empire READ get_empire NOTIFY empire_changed)
	Q_PROPERTY(metternich::landed_title* de_jure_empire READ get_de_jure_empire NOTIFY de_jure_empire_changed)
	Q_PROPERTY(metternich::world* primary_star READ get_primary_star CONSTANT)
	Q_PROPERTY(QVariantList worlds READ get_worlds_qvariant_list CONSTANT)
	Q_PROPERTY(QVariantList territory_polygon READ get_territory_polygon_qvariant_list CONSTANT)
	Q_PROPERTY(QRectF territory_bounding_rect READ get_territory_bounding_rect CONSTANT)
	Q_PROPERTY(QColor map_mode_color READ get_map_mode_color NOTIFY map_mode_color_changed)
	Q_PROPERTY(bool ethereal MEMBER ethereal READ is_ethereal NOTIFY ethereal_changed)

public:
	static constexpr const char *class_identifier = "star_system";
	static constexpr const char *database_folder = "star_systems";
	static inline const QColor empty_color = QColor("#f5f5dc");
	static constexpr int territory_radius = 1280; //the radius for the system's territory

	static void calculate_territory_polygons();

	star_system(const std::string &identifier);

	virtual void initialize() override;

	virtual std::string get_name() const override;

	landed_title *get_duchy() const
	{
		return this->duchy;
	}

	void set_duchy(landed_title *duchy);

	landed_title *get_kingdom() const;
	landed_title *get_de_jure_kingdom() const;
	landed_title *get_empire() const;
	landed_title *get_de_jure_empire() const;

	world *get_primary_star() const
	{
		return this->primary_star;
	}

	void calculate_primary_star();

	const QColor &get_map_mode_color() const
	{
		return this->map_mode_color;
	}

	void set_map_mode_color(const QColor &color)
	{
		if (color == this->get_map_mode_color()) {
			return;
		}

		this->map_mode_color = color;
		emit map_mode_color_changed();
	}

	const QColor &get_color_for_map_mode(const map_mode mode) const;

	void update_color_for_map_mode(const map_mode mode)
	{
		const QColor &color = this->get_color_for_map_mode(mode);
		this->set_map_mode_color(color);
	}

	QPointF get_center_pos() const;
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

	bool is_ethereal() const
	{
		return this->ethereal;
	}

	world *get_capital_world() const;

signals:
	void duchy_changed();
	void kingdom_changed();
	void de_jure_kingdom_changed();
	void empire_changed();
	void de_jure_empire_changed();
	void map_mode_color_changed();
	void ethereal_changed();

private:
	landed_title *duchy = nullptr; //the star system's corresponding duchy
	world *primary_star = nullptr;
	std::vector<world *> worlds;
	QPolygonF territory_polygon;
	QColor map_mode_color; //the color used for the system by the current map mode
	bool ethereal = false; //whether the star system is an ethereal one (i.e. Asgard's system); ethereal systems can only be entered by ethereal beings, who cannot enter non-ethereal systems, either; these impediments could be surpassed by technological or magical developments
};

}
