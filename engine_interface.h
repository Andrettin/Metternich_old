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
class engine_interface : public QObject, public singleton<engine_interface>
{
	Q_OBJECT

	Q_PROPERTY(metternich::game* game READ get_game CONSTANT)
	Q_PROPERTY(metternich::world* current_world READ get_current_world WRITE set_current_world NOTIFY current_world_changed)
	Q_PROPERTY(QVariantList worlds READ get_worlds CONSTANT)
	Q_PROPERTY(metternich::province* selected_province READ get_selected_province NOTIFY selected_province_changed)
	Q_PROPERTY(metternich::holding* selected_holding READ get_selected_holding NOTIFY selected_holding_changed)
	Q_PROPERTY(metternich::character* selected_character READ get_selected_character WRITE set_selected_character NOTIFY selected_character_changed)
	Q_PROPERTY(QString loading_message READ get_loading_message NOTIFY loading_message_changed)
	Q_PROPERTY(int map_mode READ get_map_mode WRITE set_map_mode NOTIFY map_mode_changed)
	Q_PROPERTY(QVariantList event_instances READ get_event_instances NOTIFY event_instances_changed)

public:
	engine_interface();
	~engine_interface();

	game *get_game() const;
	world *get_current_world() const;
	void set_current_world(world *world);
	QVariantList get_worlds() const;
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

	int get_map_mode() const;
	void set_map_mode(const int map_mode);
	
	QVariantList get_event_instances() const;
	void add_event_instance(qunique_ptr<event_instance> &&event_instance);
	Q_INVOKABLE void remove_event_instance(const QVariant &event_instance_variant);

signals:
	void current_world_changed();
	void selected_province_changed();
	void selected_holding_changed();
	void selected_character_changed();
	void loading_message_changed();
	void map_mode_changed();
	void event_instances_changed();

private:
	QString loading_message; //the loading message to be displayed
	character *selected_character = nullptr;
	std::vector<qunique_ptr<event_instance>> event_instances;
	mutable std::shared_mutex event_instances_mutex;
	mutable std::shared_mutex loading_message_mutex;
};

}
