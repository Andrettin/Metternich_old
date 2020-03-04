#pragma once

#include "qunique_ptr.h"
#include "singleton.h"

#include <QObject>
#include <QString>
#include <QVariant>

#include <memory>
#include <mutex>
#include <vector>

namespace metternich {

class character;
class event_instance;
class game;
class holding;
class province;
class world;

/**
**	@brief	Interface for the engine, to be used in the context of QML
*/
class engine_interface final : public QObject, public singleton<engine_interface>
{
	Q_OBJECT

	Q_PROPERTY(metternich::game* game READ get_game CONSTANT)
	Q_PROPERTY(QVariantList star_systems READ get_star_systems CONSTANT)
	Q_PROPERTY(QVariantList worlds READ get_worlds CONSTANT)
	Q_PROPERTY(QVariantList map_worlds READ get_map_worlds CONSTANT)
	Q_PROPERTY(QVariantList cosmic_duchies READ get_cosmic_duchies CONSTANT)
	Q_PROPERTY(metternich::province* selected_province READ get_selected_province NOTIFY selected_province_changed)
	Q_PROPERTY(metternich::holding* selected_holding READ get_selected_holding NOTIFY selected_holding_changed)
	Q_PROPERTY(metternich::character* selected_character READ get_selected_character WRITE set_selected_character NOTIFY selected_character_changed)
	Q_PROPERTY(QRectF cosmic_map_bounding_rect READ get_cosmic_map_bounding_rect CONSTANT)
	Q_PROPERTY(QString loading_message READ get_loading_message NOTIFY loading_message_changed)
	Q_PROPERTY(bool paused READ is_paused WRITE set_paused NOTIFY paused_changed)
	Q_PROPERTY(int map_mode READ get_map_mode WRITE set_map_mode NOTIFY map_mode_changed)
	Q_PROPERTY(QVariantList event_instances READ get_event_instances NOTIFY event_instances_changed)
	Q_PROPERTY(QStringList notifications READ get_notifications NOTIFY notifications_changed)
	Q_PROPERTY(QString current_notification READ get_current_notification NOTIFY current_notification_changed)

public:
	static constexpr int max_notifications = 10;

	engine_interface();
	~engine_interface();

	game *get_game() const;
	QVariantList get_star_systems() const;
	QVariantList get_worlds() const;
	QVariantList get_map_worlds() const;
	QVariantList get_cosmic_duchies() const;
	province *get_selected_province() const;
	holding *get_selected_holding() const;

	character *get_selected_character() const
	{
		return this->selected_character;
	}

	void set_selected_character(character *character)
	{
		if (character == this->get_selected_character()) {
			return;
		}

		this->selected_character = character;
		emit selected_character_changed();
	}

	const QRectF &get_cosmic_map_bounding_rect() const;

	const QString &get_loading_message() const
	{
		std::shared_lock<std::shared_mutex> lock(this->loading_message_mutex);
		return this->loading_message;
	}

	void set_loading_message(const QString &loading_message)
	{
		{
			std::unique_lock<std::shared_mutex> lock(this->loading_message_mutex);

			if (loading_message == this->loading_message) {
				return;
			}

			this->loading_message = loading_message;
		}

		emit loading_message_changed();
	}

	bool is_paused() const;
	void set_paused(const bool paused);
	int get_map_mode() const;
	void set_map_mode(const int map_mode);
	
	QVariantList get_event_instances() const;
	void add_event_instance(qunique_ptr<event_instance> &&event_instance);
	Q_INVOKABLE void remove_event_instance(const QVariant &event_instance_variant);

	const QStringList &get_notifications() const
	{
		return this->notifications;
	}

	void add_notification(const QString &notification)
	{
		this->notifications.push_back(notification);

		if (this->notifications.size() > engine_interface::max_notifications) {
			this->notifications.pop_front();
		}

		emit notifications_changed();

		this->set_current_notification(notification);
	}

	void add_notification(const std::string &notification)
	{
		this->add_notification(QString::fromStdString(notification));
	}

	void add_notification(const char *notification)
	{
		this->add_notification(QString(notification));
	}

	const QString &get_current_notification() const
	{
		return this->current_notification;
	}

	void set_current_notification(const QString &notification)
	{
		if (notification == this->get_current_notification()) {
			return;
		}

		this->current_notification = notification;
		emit current_notification_changed();
	}

signals:
	void selected_province_changed();
	void selected_holding_changed();
	void selected_character_changed();
	void loading_message_changed();
	void paused_changed();
	void map_mode_changed();
	void event_instances_changed();
	void notifications_changed();
	void current_notification_changed();

private:
	QString loading_message; //the loading message to be displayed
	character *selected_character = nullptr;
	std::vector<qunique_ptr<event_instance>> event_instances;
	QStringList notifications;
	QString current_notification;
	mutable std::shared_mutex event_instances_mutex;
	mutable std::shared_mutex loading_message_mutex;
};

}
