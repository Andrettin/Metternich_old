#pragma once

#include <QObject>
#include <QString>
#include <QVariant>

#include <memory>
#include <mutex>

namespace Metternich {

class Game;
class Province;

/**
**	@brief	Interface for the engine, to be used in the context of QML
*/
class EngineInterface : public QObject
{
	Q_OBJECT

	Q_PROPERTY(Metternich::Game* game READ GetGame CONSTANT)
	Q_PROPERTY(QVariant provinces READ GetProvinces CONSTANT)
	Q_PROPERTY(Metternich::Province* selected_province READ GetSelectedProvince NOTIFY SelectedProvinceChanged)

public:
	static EngineInterface *GetInstance();

private:
	static inline std::unique_ptr<EngineInterface> Instance;
	static inline std::once_flag OnceFlag;

public:
	Game *GetGame() const;
	QVariant GetProvinces() const;
	Province *GetSelectedProvince() const;

signals:
	void SelectedProvinceChanged();
};

}
