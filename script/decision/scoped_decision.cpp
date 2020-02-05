#include "script/decision/scoped_decision.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "script/condition/and_condition.h"
#include "script/effect/effect.h"
#include "script/effect/effect_list.h"

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
void scoped_decision<T>::do_effects(T *scope, character *source) const
{
	Q_UNUSED(source)

	if (!this->check_conditions(scope)) {
		return;
	}

	if (this->effects != nullptr) {
		this->effects->do_effects(scope);
	}
}

template <typename T>
std::string scoped_decision<T>::get_effects_string(const T *scope) const
{
	if (this->effects != nullptr) {
		return this->effects->get_effects_string(scope);
	}

	return no_effect_string;
}

template class scoped_decision<holding>;

}
