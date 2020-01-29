#include "script/effect/effect.h"

#include "character/character.h"
#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_operator.h"
#include "database/gsml_property.h"
#include "holding/holding.h"
#include "map/province.h"
#include "script/effect/combat_effect.h"
#include "script/effect/event_effect.h"
#include "script/effect/flags_effect.h"
#include "script/effect/random_list_effect.h"
#include "script/effect/traits_effect.h"
#include "script/effect/wealth_effect.h"

namespace metternich {

template <typename T>
std::unique_ptr<effect<T>> effect<T>::from_gsml_property(const gsml_property &property)
{
	const std::string &effect_identifier = property.get_key();

	if constexpr (std::is_same_v<T, character>) {
		if (effect_identifier == "traits") {
			return std::make_unique<traits_effect<T>>(property.get_value(), property.get_operator());
		} else if (effect_identifier == "wealth") {
			return std::make_unique<wealth_effect<T>>(property.get_value(), property.get_operator());
		}
	}

	if constexpr (std::is_same_v<T, character>) {
		if (effect_identifier == "event") {
			return std::make_unique<event_effect<T>>(property.get_value(), property.get_operator());
		} else if (effect_identifier == "flags") {
			return std::make_unique<flags_effect<T>>(property.get_value(), property.get_operator());
		}
	}

	throw std::runtime_error("Invalid property effect: \"" + effect_identifier + "\".");
}

template <typename T>
std::unique_ptr<effect<T>> effect<T>::from_gsml_scope(const gsml_data &scope)
{
	const std::string &effect_identifier = scope.get_tag();
	std::unique_ptr<effect<T>> effect;

	if (effect_identifier == "random_list") {
		effect = std::make_unique<random_list_effect<T>>(scope.get_operator());
	} else {
		if constexpr (std::is_same_v<T, character>) {
			if (effect_identifier == "combat") {
				effect = std::make_unique<combat_effect<T>>(scope.get_operator());
			}
		}
	}

	if (effect == nullptr) {
		throw std::runtime_error("Invalid scope effect: \"" + effect_identifier + "\".");
	}

	database::process_gsml_data(effect, scope);

	return effect;
}

template <typename T>
effect<T>::effect(const gsml_operator effect_operator) : effect_operator(effect_operator)
{
}

template <typename T>
void effect<T>::process_gsml_property(const gsml_property &property)
{
	throw std::runtime_error("Invalid property for \"" + this->get_identifier() + "\" effect: \"" + property.get_key() + "\".");
}

template <typename T>
void effect<T>::process_gsml_scope(const gsml_data &scope)
{
	throw std::runtime_error("Invalid scope for \"" + this->get_identifier() + "\" effect: \"" + scope.get_tag() + "\".");
}

template <typename T>
void effect<T>::do_effect(T *scope) const
{
	switch (this->get_operator()) {
		case gsml_operator::assignment:
			this->do_assignment_effect(scope);
			break;
		case gsml_operator::addition:
			this->do_addition_effect(scope);
			break;
		case gsml_operator::subtraction:
			this->do_subtraction_effect(scope);
			break;
		default:
			throw std::runtime_error("Invalid operator (\"" + std::to_string(static_cast<int>(this->get_operator())) + "\") for effect.");
	}
}

template <typename T>
std::string effect<T>::get_string(const T *scope, const size_t indent) const
{
	/*
	std::string scope_name;
	if constexpr (std::is_same_v<T, character> || std::is_same_v<T, holding>) {
		scope_name = scope->get_titled_name();
	} else {
		scope_name = scope->get_name();
	}

	std::string str = string::highlight(scope_name) + ": ";
	*/

	switch (this->get_operator()) {
		case gsml_operator::assignment:
			return this->get_assignment_string(scope, indent);
			break;
		case gsml_operator::addition:
			return this->get_addition_string();
			break;
		case gsml_operator::subtraction:
			return this->get_subtraction_string();
			break;
		default:
			throw std::runtime_error("Invalid operator (\"" + std::to_string(static_cast<int>(this->get_operator())) + "\") for effect.");
	}
}

template class effect<character>;
template class effect<holding>;
template class effect<province>;

}
