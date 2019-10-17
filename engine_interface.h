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
class province;

/**
**	@brief	Interface for the engine, to be used in the context of QML
*/
class EngineInterface : public QObject, public singleton<EngineInterface>
{
	Q_OBJECT

	Q_PROPERTY(metternich::Game* game READ GetGame CONSTANT)
	Q_PROPERTY(QVariantList provinces READ get_provinces CONSTANT)
	Q_PROPERTY(QVariantList river_provinces READ get_river_provinces CONSTANT)
	Q_PROPERTY(metternich::province* selected_province READ get_selected_province NOTIFY selected_province_changed)
	Q_PROPERTY(metternich::holding* selected_holding READ get_selected_holding NOTIFY selected_holding_changed)
	Q_PROPERTY(QString loading_message READ get_loading_message NOTIFY loading_message_changed)

public:
	Game *GetGame() const;
	QVariantList get_provinces() const;
	QVariantList get_river_provinces() const;
	province *get_selected_province() const;
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

	Q_INVOKABLE QPoint coordinate_to_point(const QGeoCoordinate &coordinate) const;

signals:
	void selected_province_changed();
	void selected_holding_changed();
	void loading_message_changed();

private:
	QString loading_message; //the loading message to be displayed
};

}
