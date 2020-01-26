#include "script/event/event_instance.h"

#include "game/game.h"
#include "util/container_util.h"

namespace metternich {

event_instance::event_instance(const QString &name, const QString &description, std::vector<qunique_ptr<event_option_instance>> &&options)
	: name(name), description(description), options(std::move(options))
{
}

event_instance::~event_instance()
{
}

QVariantList event_instance::get_options() const
{
	std::vector<event_option_instance *> options;
	
	for (const qunique_ptr<event_option_instance> &option : this->options) {
		options.push_back(option.get());
	}
	
	return container::to_qvariant_list(options);
}

void event_option_instance::do_effects() const
{
	game::get()->post_order(this->option_effects);
}

}
