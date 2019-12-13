#include "script/event/event_option.h"

#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "script/effect/effect.h"
#include "translator.h"

namespace metternich {

template <typename T>
event_option<T>::event_option()
{
}

template <typename T>
event_option<T>::~event_option()
{
}

template <typename T>
std::string event_option<T>::get_name() const
{
	return translator::get()->translate(this->name_tag);
}

template <typename T>
void event_option<T>::process_gsml_property(const gsml_property &property)
{
	if (property.get_key() == "name") {
		this->name_tag = property.get_value();
	} else {
		throw std::runtime_error("Invalid event option property: \"" + property.get_key() + "\".");
	}
}

template <typename T>
void event_option<T>::process_gsml_scope(const gsml_data &scope)
{
	if (scope.get_tag() == "effects") {
		for (const gsml_property &property : scope.get_properties()) {
			this->effects.push_back(effect<T>::from_gsml_property(property));
		}
	} else {
		throw std::runtime_error("Invalid event option scope: \"" + scope.get_tag() + "\".");
	}
}

template <typename T>
void event_option<T>::do_effects(T *scope) const
{
	for (const std::unique_ptr<effect<T>> &effect : this->effects) {
		effect->do_effect(scope);
	}
}

template class event_option<character>;

}
