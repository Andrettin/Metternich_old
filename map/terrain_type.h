#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QColor>
#include <QGeoPath>
#include <QGeoPolygon>

namespace metternich {

class holding;
class province;

template <typename T>
class modifier;

class terrain_type final : public data_entry, public data_type<terrain_type>
{
	Q_OBJECT

	Q_PROPERTY(QColor color MEMBER color READ get_color)
	Q_PROPERTY(bool water MEMBER water READ is_water NOTIFY water_changed)
	Q_PROPERTY(bool navigable MEMBER navigable READ is_navigable)
	Q_PROPERTY(bool ocean MEMBER ocean READ is_ocean)
	Q_PROPERTY(bool river MEMBER river READ is_river)
	Q_PROPERTY(int path_width MEMBER path_width READ get_path_width)
	Q_PROPERTY(bool default_terrain READ is_default_terrain WRITE set_default_terrain)

public:
	static constexpr const char *class_identifier = "terrain";
	static constexpr const char *database_folder = "terrain_types";
	static constexpr QRgb empty_rgb = qRgb(0, 0, 0);

	static terrain_type *get_by_rgb(const QRgb &rgb, const bool should_find = true);

	static terrain_type *get_default_terrain()
	{
		return terrain_type::default_terrain;
	}

private:
	static inline std::map<QRgb, terrain_type *> instances_by_rgb;
	static inline terrain_type *default_terrain = nullptr;

public:
	terrain_type(const std::string &identifier);
	virtual ~terrain_type() override;

	virtual void process_gsml_scope(const gsml_data &scope) override;

	const QColor &get_color() const
	{
		return this->color;
	}

	bool is_water() const
	{
		return this->water;
	}

	bool is_navigable() const
	{
		return this->navigable;
	}

	bool is_ocean() const
	{
		return this->ocean;
	}

	bool is_river() const
	{
		return this->river;
	}

	int get_path_width() const
	{
		return this->path_width;
	}

	bool is_default_terrain() const
	{
		return terrain_type::default_terrain == this;
	}

	void set_default_terrain(const bool default_terrain)
	{
		if (default_terrain) {
			terrain_type::default_terrain = this;
		} else {
			if (this->is_default_terrain())  {
				terrain_type::default_terrain = nullptr;
			}
		}
	}

	const std::unique_ptr<modifier<holding>> &get_holding_modifier() const
	{
		return this->holding_modifier;
	}

	const std::unique_ptr<modifier<province>> &get_province_modifier() const
	{
		return this->province_modifier;
	}

signals:
	void water_changed();

private:
	QColor color; //the color used to identify the terrain in the terrain map
	bool water = false; //whether the terrain is a water terrain
	bool navigable = false; //whether this water terrain is navigable
	bool ocean = false;
	bool river = false;
	int path_width = 0;
	std::unique_ptr<modifier<holding>> holding_modifier; //the modifier applied to holdings with this terrain
	std::unique_ptr<modifier<province>> province_modifier; //the modifier applied to provinces with this terrain
};

}
