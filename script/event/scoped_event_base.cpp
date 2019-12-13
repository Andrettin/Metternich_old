#include "script/event/scoped_event_base.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "random.h"
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
void scoped_event_base<T>::pick_option(T *scope) const
{
	if (this->options.empty()) {
		throw std::runtime_error("Event has no options.");
	}

	this->options[random::generate(this->options.size())]->do_effects(scope);
}

template class scoped_event_base<character>;

}
