#include "metternich.h"

#include "defines.h"
#include "province.h"

#include <QList>

Metternich *Metternich::GetInstance()
{
	std::call_once(Metternich::OnceFlag, [](){ Metternich::Instance = std::make_unique<Metternich>(); });
	
	return Metternich::Instance.get();
}

QString Metternich::GetAssetImportPath() const
{
	return QString::fromStdString(Defines::GetAssetImportPath());
}

QVariant Metternich::GetProvinces() const
{
	QObjectList province_list;
	for (Province *province : Province::GetAll()) {
		province_list.append(province);
	}
	return QVariant::fromValue(province_list);
}
