#pragma once

#include <QObject>
#include <QString>
#include <QVariant>

#include <memory>
#include <mutex>

namespace Metternich {

class Game;
class Holding;
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
	Q_PROPERTY(Metternich::Holding* selected_holding READ GetSelectedHolding NOTIFY SelectedHoldingChanged)

public:
	static EngineInterface *GetInstance();

private:
	static inline std::unique_ptr<EngineInterface> Instance;
	static inline std::once_flag OnceFlag;

public:
	Game *GetGame() const;
	QVariant GetProvinces() const;
	Province *GetSelectedProvince() const;
	Holding *GetSelectedHolding() const;

signals:
	void SelectedProvinceChanged();
	void SelectedHoldingChanged();
};

}
