#include "script/event/scoped_event_base.h"

#include "character/character.h"
#include "database/database.h"
#include "database/gsml_data.h"
#include "game/engine_interface.h"
#include "script/chance_util.h"
#include "script/condition/and_condition.h"
#include "script/context.h"
#include "script/effect/effect.h"
#include "script/effect/effect_list.h"
#include "script/event/event_instance.h"
#include "script/event/event_option.h"
#include "script/event/event_trigger.h"
#include "util/random.h"
#include "util/string_util.h"
#include "util/translator.h"

#include <QString>

#include <functional>

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
void scoped_event_base<T>::initialize()
{
	for (event_trigger<T> *trigger : this->triggers) {
		if (this->random) {
			trigger->add_random_event(this);
		} else {
			trigger->add_event(this);
		}
	}
}

template <typename T>
void scoped_event_base<T>::process_gsml_property(const gsml_property &property)
{
	if (property.get_key() == "triggers") {
		event_trigger<T> *trigger = event_trigger<T>::get(property.get_value());
		switch (property.get_operator()) {
			case gsml_operator::addition:
				this->triggers.insert(trigger);
				break;
			case gsml_operator::subtraction:
				this->triggers.erase(trigger);
				break;
			default:
				throw std::runtime_error("Invalid operator for \"" + property.get_key() + "\" event property.");
		}
	} else if (property.get_key() == "random") {
		switch (property.get_operator()) {
			case gsml_operator::assignment:
				this->random = string::to_bool(property.get_value());
				break;
			default:
				throw std::runtime_error("Invalid operator for \"" + property.get_key() + "\" event property.");
		}
	} else if (property.get_key() == "hidden") {
		switch (property.get_operator()) {
			case gsml_operator::assignment:
				this->hidden = string::to_bool(property.get_value());
				break;
			default:
				throw std::runtime_error("Invalid operator for \"" + property.get_key() + "\" event property.");
		}
	} else {
		throw std::runtime_error("Invalid event property: \"" + property.get_key() + "\".");
	}
}

template <typename T>
void scoped_event_base<T>::process_gsml_scope(const gsml_data &scope)
{
	if (scope.get_tag() == "conditions") {
		this->conditions = std::make_unique<and_condition<T>>();
		database::process_gsml_data(this->conditions.get(), scope);
	} else if (scope.get_tag() == "immediate_effects") {
		this->immediate_effects = std::make_unique<effect_list<T>>();
		database::process_gsml_data(this->immediate_effects, scope);
	} else if (scope.get_tag() == "option") {
		auto option = std::make_unique<event_option<T>>();
		database::process_gsml_data(option.get(), scope);
		this->options.push_back(std::move(option));
	} else {
		throw std::runtime_error("Invalid event scope: \"" + scope.get_tag() + "\".");
	}
}

template <typename T>
std::string scoped_event_base<T>::get_title() const
{
	return translator::get()->translate(this->get_tag());
}

template <typename T>
std::string scoped_event_base<T>::get_description() const
{
	return translator::get()->translate(this->get_tag() + "_desc");
}

template <typename T>
bool scoped_event_base<T>::check_conditions(const T *scope, const read_only_context &ctx) const
{
	if (this->conditions == nullptr) {
		return true;
	}

	return this->conditions->check(scope, ctx);
}

template <typename T>
void scoped_event_base<T>::do_event(T *scope, const context &ctx) const
{
	if (this->immediate_effects != nullptr) {
		this->immediate_effects->do_effects(scope, ctx);
	}

	if (scope->is_ai() || this->hidden) {
		this->pick_option(scope, ctx);
	} else {
		//add event to the list of events to be shown to the player
		std::vector<qunique_ptr<event_option_instance>> option_instances;
		for (const std::unique_ptr<event_option<T>> &option : this->options) {
			const event_option<T> *option_ptr = option.get();
			std::function<void()> option_effects = [option_ptr, scope, ctx]() {
				option_ptr->do_effects(scope, ctx);
			};

			const std::string effects_string = option->get_effects_string(scope, ctx);
			auto option_instance = make_qunique<event_option_instance>(QString::fromStdString(option->get_name()), string::to_tooltip(effects_string), option_effects);
			option_instance->moveToThread(QApplication::instance()->thread());
			option_instances.push_back(std::move(option_instance));
		}
		
		auto evt_instance = make_qunique<event_instance>(QString::fromStdString(this->get_title()), QString::fromStdString(this->get_description()), std::move(option_instances));
		evt_instance->moveToThread(QApplication::instance()->thread());
		engine_interface::get()->add_event_instance(std::move(evt_instance));
	}
}

template <typename T>
void scoped_event_base<T>::pick_option(T *scope, const context &ctx) const
{
	if (this->options.empty()) {
		return;
	}

	std::map<const event_option<T> *, const chance_factor<T> *> option_ai_chances;
	for (const std::unique_ptr<event_option<T>> &option : this->options) {
		if (option->get_ai_chance_factor() == nullptr) {
			throw std::runtime_error("Event option has no AI chance factor.");
		}

		option_ai_chances[option.get()] = option->get_ai_chance_factor();
	}

	const event_option<T> *option = calculate_chance_list_result(option_ai_chances, scope);
	option->do_effects(scope, ctx);
}

template class scoped_event_base<character>;

}
