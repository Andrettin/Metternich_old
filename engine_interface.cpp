#include "engine_interface.h"

#include "defines.h"
#include "game/game.h"
#include "holding/holding.h"
#include "map/province.h"
#include "util.h"

#include <QList>

namespace metternich {

Game *EngineInterface::GetGame() const
{
	return Game::Get();
}

QVariant EngineInterface::GetProvinces() const
{
	QObjectList province_list = VectorToQObjectList(Province::GetAll());
	return QVariant::fromValue(province_list);
}

Province *EngineInterface::GetSelectedProvince() const
{
	return Province::GetSelectedProvince();
}

Holding *EngineInterface::GetSelectedHolding() const
{
	return Holding::GetSelectedHolding();
}

}
