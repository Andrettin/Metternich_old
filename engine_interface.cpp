#include "engine_interface.h"

#include "defines.h"
#include "game/game.h"
#include "holding/holding.h"
#include "map/map.h"
#include "map/province.h"
#include "util/container_util.h"

#include <QList>

namespace metternich {

game *engine_interface::get_game() const
{
	return game::get();
}

QVariantList engine_interface::get_provinces() const
{
	QVariantList province_list = util::container_to_qvariant_list(province::get_all());
	return province_list;
}

QVariantList engine_interface::get_river_provinces() const
{
	QVariantList province_list = util::container_to_qvariant_list(province::get_river_provinces());
	return province_list;
}

province *engine_interface::get_selected_province() const
{
	return province::get_selected_province();
}

holding *engine_interface::get_selected_holding() const
{
	return holding::get_selected_holding();
}

/**
**	@brief	Convert a coordinate to a pixel position on the map
**
**	@param	coordinate	The geocoordinate
**
**	@return The pixel position corresponding to the coordinate
*/
QPoint engine_interface::coordinate_to_point(const QGeoCoordinate &coordinate) const
{
	return map::get()->get_coordinate_pos(coordinate);
}

/**
**	@brief	Set the map mode
**
**	@param	map_mode	The new map mode
*/
void engine_interface::set_map_mode(const int map_mode)
{
	game::get()->post_order([map_mode]() {
		map::get()->set_mode(static_cast<metternich::map_mode>(map_mode));
	});
}

}
