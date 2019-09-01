#pragma once

#include "singleton.h"

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
class EngineInterface : public QObject, public Singleton<EngineInterface>
{
	Q_OBJECT

	Q_PROPERTY(Metternich::Game* game READ GetGame CONSTANT)
	Q_PROPERTY(QVariant provinces READ GetProvinces CONSTANT)
	Q_PROPERTY(Metternich::Province* selected_province READ GetSelectedProvince NOTIFY SelectedProvinceChanged)
	Q_PROPERTY(Metternich::Holding* selected_holding READ GetSelectedHolding NOTIFY SelectedHoldingChanged)
	Q_PROPERTY(QString loading_message READ GetLoadingMessage NOTIFY LoadingMessageChanged)

public:
	Game *GetGame() const;
	QVariant GetProvinces() const;
	Province *GetSelectedProvince() const;
	Holding *GetSelectedHolding() const;

	const QString &GetLoadingMessage() const
	{
		return this->LoadingMessage;
	}

	void SetLoadingMessage(const QString &loading_message)
	{
		if (loading_message == this->GetLoadingMessage()) {
			return;
		}

		this->LoadingMessage = loading_message;
		emit LoadingMessageChanged();
	}

signals:
	void SelectedProvinceChanged();
	void SelectedHoldingChanged();
	void LoadingMessageChanged();

private:
	QString LoadingMessage; //the loading message to be displayed
};

}
