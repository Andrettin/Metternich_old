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

QVariant EngineInterface::get_provinces() const
{
	QObjectList province_list = util::vector_to_qobject_list(province::get_all());
	return QVariant::fromValue(province_list);
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
