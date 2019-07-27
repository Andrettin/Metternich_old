#include "metternich.h"

#include "defines.h"

Metternich *Metternich::GetInstance()
{
	std::call_once(Metternich::OnceFlag, [](){ Metternich::Instance = std::make_unique<Metternich>(); });
	
	return Metternich::Instance.get();
}

QString Metternich::GetAssetImportPath() const
{
	return QString::fromStdString(Defines::GetAssetImportPath());
}
