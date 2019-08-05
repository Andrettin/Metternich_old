#include "engine_interface.h"

#include "defines.h"
#include "game.h"
#include "map/province.h"
#include "util.h"

#include <QList>

namespace Metternich {

EngineInterface *EngineInterface::GetInstance()
{
	std::call_once(EngineInterface::OnceFlag, [](){ EngineInterface::Instance = std::make_unique<EngineInterface>(); });
	
	return EngineInterface::Instance.get();
}

Game *EngineInterface::GetGame() const
{
	return Game::GetInstance();
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

}
