#include "engine_interface.h"

#include "defines.h"
#include "game/game.h"
#include "holding/holding.h"
#include "map/map.h"
#include "map/province.h"
#include "map/world.h"
#include "script/event/event_instance.h"
#include "util/container_util.h"

#include <QList>

namespace metternich {

engine_interface::engine_interface()
{
	connect(game::get(), &game::paused_changed, this, &engine_interface::paused_changed);
}

engine_interface::~engine_interface()
{
}

game *engine_interface::get_game() const
{
	return game::get();
}

world *engine_interface::get_current_world() const
{
	return map::get()->get_current_world();
}

void engine_interface::set_current_world(world *world)
{
	game::get()->post_order([world]() {
		map::get()->set_current_world(world);
	});
}

QVariantList engine_interface::get_worlds() const
{
	QVariantList world_list = container::to_qvariant_list(world::get_all());
	return world_list;
}

QVariantList engine_interface::get_map_worlds() const
{
	QVariantList world_list = container::to_qvariant_list(world::get_map_worlds());
	return world_list;
}

province *engine_interface::get_selected_province() const
{
	return province::get_selected_province();
}

holding *engine_interface::get_selected_holding() const
{
	return holding::get_selected_holding();
}

bool engine_interface::is_paused() const
{
	return game::get()->is_paused();
}

void engine_interface::set_paused(const bool paused)
{
	game::get()->post_order([paused]() {
		game::get()->set_paused(paused);
	});
}

int engine_interface::get_map_mode() const
{
	return static_cast<int>(map::get()->get_mode());
}

void engine_interface::set_map_mode(const int map_mode)
{
	game::get()->post_order([map_mode]() {
		map::get()->set_mode(static_cast<metternich::map_mode>(map_mode));
	});
}

QVariantList engine_interface::get_event_instances() const
{
	std::shared_lock<std::shared_mutex> lock(this->event_instances_mutex);
	
	std::vector<event_instance *> event_instances;
	
	for (const qunique_ptr<event_instance> &event_instance : this->event_instances) {
		event_instances.push_back(event_instance.get());
	}
	
	return container::to_qvariant_list(event_instances);
}

void engine_interface::add_event_instance(qunique_ptr<event_instance> &&event_instance)
{
	{
		std::unique_lock<std::shared_mutex> lock(this->event_instances_mutex);

		if (this->event_instances.empty()) {
			game::get()->post_order([]() {
				game::get()->set_paused(true);
			});
		}

		this->event_instances.push_back(std::move(event_instance));
	}

	emit event_instances_changed();
}

void engine_interface::remove_event_instance(const QVariant &event_instance_variant)
{
	{
		std::unique_lock<std::shared_mutex> lock(this->event_instances_mutex);

		QObject *object = qvariant_cast<QObject *>(event_instance_variant);
		event_instance *event_instance_to_remove = static_cast<event_instance *>(object);

		for (size_t i = 0; i < this->event_instances.size(); ++i) {
			event_instance *event_instance = this->event_instances[i].get();
			if (event_instance == event_instance_to_remove) {
				this->event_instances.erase(this->event_instances.begin() + static_cast<int>(i));
				break;
			}
		}

		if (this->event_instances.empty()) {
			game::get()->post_order([]() {
				game::get()->set_paused(false);
			});
		}
	}

	emit event_instances_changed();
}

}
