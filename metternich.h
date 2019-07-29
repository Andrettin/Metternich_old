#pragma once

#include <QObject>
#include <QString>
#include <QVariant>

#include <memory>
#include <mutex>

class Game;
class Province;

/**
**	@brief	Interface for the engine, to be used in the context of QML
*/
class Metternich : public QObject
{
	Q_OBJECT

	Q_PROPERTY(Game* game READ GetGame CONSTANT)
	Q_PROPERTY(QString asset_import_path READ GetAssetImportPath CONSTANT)
	Q_PROPERTY(QVariant provinces READ GetProvinces CONSTANT)
	Q_PROPERTY(Province* selected_province READ GetSelectedProvince NOTIFY SelectedProvinceChanged)

public:
	static Metternich *GetInstance();

private:
	static inline std::unique_ptr<Metternich> Instance;
	static inline std::once_flag OnceFlag;

public:
	Game *GetGame() const;
	QString GetAssetImportPath() const;
	QVariant GetProvinces() const;
	Province *GetSelectedProvince() const;

signals:
	void SelectedProvinceChanged();
};
