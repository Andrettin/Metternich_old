#include "script/event/event_option.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "script/chance_factor.h"
#include "script/effect/effect.h"
#include "script/effect/effect_list.h"
#include "translator.h"

namespace metternich {

template <typename T>
event_option<T>::event_option()
{
	//the default AI chance for event options is a simple factor of 1
	this->ai_chance = std::make_unique<chance_factor<T>>(1);
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
	const std::string &tag = scope.get_tag();

	if (tag == "ai_chance") {
		this->ai_chance = std::make_unique<chance_factor<T>>();
		database::process_gsml_data(this->ai_chance, scope);
	} else if (tag == "effects") {
		this->effects = std::make_unique<effect_list<T>>();
		database::process_gsml_data(this->effects, scope);
	} else {
		throw std::runtime_error("Invalid event option scope: \"" + scope.get_tag() + "\".");
	}
}

template <typename T>
void event_option<T>::do_effects(T *scope, const context &ctx) const
{
	if (this->effects == nullptr) {
		return;
	}

	this->effects->do_effects(scope, ctx);
}

template <typename T>
std::string event_option<T>::get_effects_string(const T *scope, const context &ctx) const
{
	if (this->effects != nullptr) {
		std::string effects_string = this->effects->get_effects_string(scope, ctx);
		if (!effects_string.empty()) {
			return effects_string;
		}
	}

	return no_effect_string;
}

template class event_option<character>;

}
