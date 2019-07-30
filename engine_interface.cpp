#include "engine_interface.h"

#include "defines.h"
#include "game.h"
#include "map/province.h"

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

QString EngineInterface::GetAssetImportPath() const
{
	return QString::fromStdString(Defines::GetAssetImportPath());
}

QVariant EngineInterface::GetProvinces() const
{
	QObjectList province_list;
	for (Province *province : Province::GetAll()) {
		province_list.append(province);
	}
	return QVariant::fromValue(province_list);
}

Province *EngineInterface::GetSelectedProvince() const
{
	return Province::GetSelectedProvince();
}

}
