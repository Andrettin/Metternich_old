#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QColor>

namespace metternich {

class Modifier;

class terrain_type : public data_entry, public data_type<terrain_type>
{
	Q_OBJECT

	Q_PROPERTY(QColor color READ get_color CONSTANT)
	Q_PROPERTY(bool water MEMBER water READ is_water NOTIFY water_changed)
	Q_PROPERTY(bool navigable MEMBER navigable READ is_navigable)
	Q_PROPERTY(bool ocean MEMBER ocean READ is_ocean)
	Q_PROPERTY(bool river MEMBER river READ is_river)

public:
	static constexpr const char *class_identifier = "terrain";
	static constexpr const char *database_folder = "terrain_types";

	static terrain_type *get_by_rgb(const QRgb &rgb, const bool should_find = true);

	/**
	**	@brief	Process the map database for the terrain type
	*/
	static void process_map_database()
	{
		std::filesystem::path map_path("./map/" + std::string(terrain_type::database_folder));

		if (!std::filesystem::exists(map_path)) {
			return;
		}

		std::filesystem::directory_iterator dir_iterator(map_path);

		for (const std::filesystem::directory_entry &dir_entry : dir_iterator) {
			terrain_type *terrain = terrain_type::get(dir_entry.path().stem().string());

			if (dir_entry.is_directory()) {
				std::filesystem::recursive_directory_iterator subdir_iterator(dir_entry);

				for (const std::filesystem::directory_entry &subdir_entry : subdir_iterator) {
					if (!subdir_entry.is_regular_file() || subdir_entry.path().extension() != ".txt") {
						continue;
					}

					gsml_parser parser(subdir_entry.path());
					database::process_gsml_data(terrain, parser.parse());
				}
			} else if (dir_entry.is_regular_file() && dir_entry.path().extension() == ".txt") {
				gsml_parser parser(dir_entry.path());
				database::process_gsml_data(terrain, parser.parse());
			}
		}
	}

private:
	static inline std::map<QRgb, terrain_type *> instances_by_rgb;

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

	const std::unique_ptr<metternich::Modifier> &get_modifier() const
	{
		return this->modifier;
	}

	const std::vector<QGeoPolygon> &get_geopolygons() const
	{
		return this->geopolygons;
	}

	bool contains_coordinate(const QGeoCoordinate &coordinate) const
	{
		//get whether the areas of this terrain type contain the given coordinate
		for (const QGeoPolygon &geopolygon : this->geopolygons) {
			if (geopolygon.contains(coordinate)) {
				return true;
			}
		}

		return false;
	}

signals:
	void water_changed();

private:
	QColor color; //the color used to identify the terrain in the terrain map
	bool water = false; //whether the terrain is a water terrain
	bool navigable = false; //whether this water terrain is navigable
	bool ocean = false;
	bool river = false;
	std::unique_ptr<metternich::Modifier> modifier; //the modifier applied to provinces with this terrain
	std::vector<QGeoPolygon> geopolygons;
};

}
