#include "script/event/scoped_event_base.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "random.h"
#include "script/chance_util.h"
#include "script/condition/and_condition.h"
#include "script/event/event_option.h"

namespace metternich {

template <typename T>
scoped_event_base<T>::scoped_event_base()
{
}

template <typename T>
scoped_event_base<T>::~scoped_event_base()
{
}

template <typename T>
void scoped_event_base<T>::process_gsml_scope(const gsml_data &scope)
{
	if (scope.get_tag() == "conditions") {
		this->conditions = std::make_unique<and_condition<character>>();
		database::process_gsml_data(this->conditions.get(), scope);
	} else if (scope.get_tag() == "option") {
		auto option = std::make_unique<event_option<character>>();
		database::process_gsml_data(option.get(), scope);
		this->options.push_back(std::move(option));
	} else {
		throw std::runtime_error("Invalid event scope: \"" + scope.get_tag() + "\".");
	}
}

template <typename T>
bool scoped_event_base<T>::check_conditions(const T *scope) const
{
	if (this->conditions == nullptr) {
		return true;
	}

	return this->conditions->check(scope);
}

template <typename T>
void scoped_event_base<T>::do_event(T *scope) const
{
	this->pick_option(scope);
}

template <typename T>
void scoped_event_base<T>::pick_option(T *scope) const
{
	if (this->options.empty()) {
		throw std::runtime_error("Event has no options.");
	}

	std::map<const event_option<T> *, const chance_factor<T> *> option_ai_chances;
	for (const std::unique_ptr<event_option<T>> &option : this->options) {
		if (option->get_ai_chance_factor() == nullptr) {
			throw std::runtime_error("Event option has no AI chance factor.");
		}

		option_ai_chances[option.get()] = option->get_ai_chance_factor();
	}

	const event_option<T> *option = calculate_chance_list_result(option_ai_chances, scope);
	option->do_effects(scope);
}

template class scoped_event_base<character>;

}
