#include "script/event/event_trigger.h"

#include "script/context.h"
#include "script/event/scoped_event_base.h"
#include "util/vector_random_util.h"

namespace metternich {

template <typename T>
void event_trigger<T>::do_events(T *scope, const context &ctx) const
{
	for (const auto *event : this->events) {
		if (event->check_conditions(scope, ctx)) {
			event->do_event(scope, ctx);
		}
	}

	std::vector<const scoped_event_base<T> *> random_events;
	random_events.reserve(this->random_events.size());
	for (const auto *event : this->random_events) {
		if (event->check_conditions(scope, ctx)) {
			random_events.push_back(event);
		}
	}
	if (!random_events.empty()) {
		vector::get_random(random_events)->do_event(scope, ctx);
	}
}

template <typename T>
void event_trigger<T>::do_events(T *scope) const
{
	context ctx;
	if constexpr (std::is_same_v<T, character>) {
		ctx.current_character = scope;
	}
	this->do_events(scope, ctx);
}

template class event_trigger<character>;

}
