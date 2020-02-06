#include "script/decision/scoped_decision.h"

#include "character/character.h"
#include "database/database.h"
#include "database/gsml_data.h"
#include "script/condition/and_condition.h"
#include "script/context.h"
#include "script/effect/effect.h"
#include "script/effect/effect_list.h"
#include "util/string_util.h"

namespace metternich {

template <typename T>
scoped_decision<T>::scoped_decision()
{
}

template <typename T>
scoped_decision<T>::~scoped_decision()
{
}

template <typename T>
void scoped_decision<T>::process_gsml_scope(const gsml_data &scope)
{
	if (scope.get_tag() == "preconditions") {
		this->preconditions = std::make_unique<and_condition<T>>();
		database::process_gsml_data(this->preconditions.get(), scope);
	} else if (scope.get_tag() == "conditions") {
		this->conditions = std::make_unique<and_condition<T>>();
		database::process_gsml_data(this->conditions.get(), scope);
	} else if (scope.get_tag() == "source_preconditions") {
		this->source_preconditions = std::make_unique<and_condition<character>>();
		database::process_gsml_data(this->source_preconditions.get(), scope);
	} else if (scope.get_tag() == "source_conditions") {
		this->source_conditions = std::make_unique<and_condition<character>>();
		database::process_gsml_data(this->source_conditions.get(), scope);
	} else if (scope.get_tag() == "effects") {
		this->effects = std::make_unique<effect_list<T>>();
		database::process_gsml_data(this->effects, scope);
	} else {
		throw std::runtime_error("Invalid decision scope: \"" + scope.get_tag() + "\".");
	}
}

template <typename T>
bool scoped_decision<T>::check_preconditions(const T *scope) const
{
	if (this->preconditions == nullptr) {
		return true;
	}

	return this->preconditions->check(scope);
}

template <typename T>
bool scoped_decision<T>::check_preconditions(const T *scope, const character *source) const
{
	return this->check_preconditions(scope) && this->check_source_preconditions(source);
}

template <typename T>
bool scoped_decision<T>::check_conditions(const T *scope) const
{
	if (!this->check_preconditions(scope)) {
		return false;
	}

	if (this->conditions == nullptr) {
		return true;
	}

	return this->conditions->check(scope);
}

template <typename T>
bool scoped_decision<T>::check_conditions(const T *scope, const character *source) const
{
	return this->check_conditions(scope) && this->check_source_conditions(source);
}

template <typename T>
bool scoped_decision<T>::check_source_preconditions(const character *source) const
{
	if (this->source_preconditions == nullptr) {
		return true;
	}

	return this->source_preconditions->check(source);
}

template <typename T>
bool scoped_decision<T>::check_source_conditions(const character *source) const
{
	if (!this->check_source_preconditions(source)) {
		return false;
	}

	if (this->source_conditions == nullptr) {
		return true;
	}

	return this->source_conditions->check(source);
}

template <typename T>
void scoped_decision<T>::do_effects(T *scope, character *source) const
{
	if (this->effects != nullptr) {
		context ctx;
		ctx.source_character = source;
		this->effects->do_effects(scope, ctx);
	}
}

template <typename T>
QString scoped_decision<T>::get_string(const T *scope, character *source) const
{

	context conditions_ctx;
	std::string conditions_string;

	if (this->conditions != nullptr) {
		conditions_string += this->conditions->get_conditions_string(scope, conditions_ctx, 1);
	}

	if (this->source_conditions != nullptr) {
		const std::string source_conditions_string = this->source_conditions->get_conditions_string(source, conditions_ctx, 2);
		if (!source_conditions_string.empty()) {
			if (!conditions_string.empty()) {
				conditions_string += "\n";
			}
			conditions_string += std::string(1, '\t') + string::highlight(source->get_titled_name()) + ":\n";
			conditions_string += source_conditions_string;
		}
	}

	std::string str;
	if (!conditions_string.empty()) {
		str += "Conditions:\n" + conditions_string + "\n\n";
	}

	str += "Effects:\n";

	std::string effects_string;
	if (this->effects != nullptr) {
		context ctx;
		ctx.source_character = source;
		effects_string = this->effects->get_effects_string(scope, ctx, 1);
	}

	if (!effects_string.empty()) {
		str += effects_string;
	} else {
		str += std::string(1, '\t') + no_effect_string;
	}

	return string::to_tooltip(str);
}

template class scoped_decision<holding>;

}
