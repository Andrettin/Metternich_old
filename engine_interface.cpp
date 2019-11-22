#include "engine_interface.h"

#include "defines.h"
#include "game/game.h"
#include "holding/holding.h"
#include "map/map.h"
#include "map/province.h"
#include "map/world.h"
#include "util/container_util.h"

#include <QList>

namespace metternich {

game *engine_interface::get_game() const
{
	return game::get();
}

world *engine_interface::get_current_world() const
{
	return map::get()->get_current_world();
}

QVariantList engine_interface::get_worlds() const
{
	QVariantList world_list = util::container_to_qvariant_list(world::get_all());
	return world_list;
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
**	@brief	Get the map mode
**
**	@return	The map mode
*/
int engine_interface::get_map_mode() const
{
	return static_cast<int>(map::get()->get_mode());
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
