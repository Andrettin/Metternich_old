#pragma once

#include <QObject>
#include <QString>
#include <QVariant>

#include <memory>
#include <mutex>

class Province;

/**
**	@brief	Interface for the engine, to be used in the context of QML
*/
class Metternich : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString assetImportPath READ GetAssetImportPath CONSTANT)
	Q_PROPERTY(QVariant provinces READ GetProvinces CONSTANT)

public:
	static Metternich *GetInstance();
	
private:
	static inline std::unique_ptr<Metternich> Instance;
	static inline std::once_flag OnceFlag;

public:
	QString GetAssetImportPath() const;
	QVariant GetProvinces() const;
};
