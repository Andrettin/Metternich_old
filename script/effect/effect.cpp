#include "script/effect/effect.h"

#include "character/character.h"
#include "database/gsml_operator.h"
#include "database/gsml_property.h"
#include "holding/holding.h"
#include "map/province.h"
#include "script/effect/flags_effect.h"
#include "script/effect/traits_effect.h"

namespace metternich {

template <typename T>
std::unique_ptr<effect<T>> effect<T>::from_gsml_property(const gsml_property &property)
{
	const std::string &effect_identifier = property.get_key();

	if constexpr (std::is_same_v<T, character>) {
		if (effect_identifier == "traits") {
			return std::make_unique<traits_effect<T>>(property.get_value(), property.get_operator());
		}
	}

	if constexpr (std::is_same_v<T, character>) {
		if (effect_identifier == "flags") {
			return std::make_unique<flags_effect<T>>(property.get_value(), property.get_operator());
		}
	}

	throw std::runtime_error("Invalid property effect: \"" + effect_identifier + "\".");
}

template <typename T>
effect<T>::effect(const gsml_operator effect_operator) : effect_operator(effect_operator)
{
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
std::string effect<T>::get_string(const T *scope) const
{
	std::string scope_name;
	if constexpr (std::is_same_v<T, character> || std::is_same_v<T, holding>) {
		scope_name = scope->get_titled_name();
	} else {
		scope_name = scope->get_name();
	}

	std::string str = string::highlight(scope_name) + ": ";

	switch (this->get_operator()) {
		case gsml_operator::assignment:
			str += this->get_assignment_string();
			break;
		case gsml_operator::addition:
			str += this->get_addition_string();
			break;
		case gsml_operator::subtraction:
			str += this->get_subtraction_string();
			break;
		default:
			throw std::runtime_error("Invalid operator (\"" + std::to_string(static_cast<int>(this->get_operator())) + "\") for effect.");
	}

	return str;
}

template class effect<character>;
template class effect<holding>;
template class effect<province>;

}
