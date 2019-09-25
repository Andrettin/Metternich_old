#pragma once

#include "singleton.h"

#include <QObject>
#include <QString>
#include <QVariant>

#include <memory>
#include <mutex>

namespace metternich {

class Game;
class holding;
class Province;

/**
**	@brief	Interface for the engine, to be used in the context of QML
*/
class EngineInterface : public QObject, public singleton<EngineInterface>
{
	Q_OBJECT

	Q_PROPERTY(metternich::Game* game READ GetGame CONSTANT)
	Q_PROPERTY(QVariant provinces READ GetProvinces CONSTANT)
	Q_PROPERTY(metternich::Province* selected_province READ GetSelectedProvince NOTIFY SelectedProvinceChanged)
	Q_PROPERTY(metternich::holding* selected_holding READ get_selected_holding NOTIFY selected_holding_changed)
	Q_PROPERTY(QString loading_message READ get_loading_message NOTIFY loading_message_changed)

public:
	Game *GetGame() const;
	QVariant GetProvinces() const;
	Province *GetSelectedProvince() const;
	holding *get_selected_holding() const;

	const QString &get_loading_message() const
	{
		return this->loading_message;
	}

	void set_loading_message(const QString &loading_message)
	{
		if (loading_message == this->get_loading_message()) {
			return;
		}

		this->loading_message = loading_message;
		emit loading_message_changed();
	}

signals:
	void SelectedProvinceChanged();
	void selected_holding_changed();
	void loading_message_changed();

private:
	QString loading_message; //the loading message to be displayed
};

}
