#include "script/event/event_trigger.h"

#include "script/event/scoped_event_base.h"
#include "util/vector_random_util.h"

namespace metternich {

template <typename T>
void event_trigger<T>::do_events(T *scope) const
{
	for (const auto *event : this->events) {
		if (event->check_conditions(scope)) {
			event->do_event(scope);
		}
	}

	std::vector<const scoped_event_base<T> *> random_events;
	random_events.reserve(this->random_events.size());
	for (const auto *event : this->random_events) {
		if (event->check_conditions(scope)) {
			random_events.push_back(event);
		}
	}
	if (!random_events.empty()) {
		vector::get_random(random_events)->do_event(scope);
	}
}

template class event_trigger<character>;

}
