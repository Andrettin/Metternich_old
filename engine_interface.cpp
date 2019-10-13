#include "engine_interface.h"

#include "defines.h"
#include "game/game.h"
#include "holding/holding.h"
#include "map/province.h"
#include "util/container_util.h"

#include <QList>

namespace metternich {

Game *EngineInterface::GetGame() const
{
	return Game::get();
}

QVariantList EngineInterface::get_provinces() const
{
	QVariantList province_list = util::container_to_qvariant_list(province::get_all());
	return province_list;
}

QVariantList EngineInterface::get_river_provinces() const
{
	QVariantList province_list = util::container_to_qvariant_list(province::get_river_provinces());
	return province_list;
}

province *EngineInterface::get_selected_province() const
{
	return province::get_selected_province();
}

holding *EngineInterface::get_selected_holding() const
{
	return holding::get_selected_holding();
}

}
