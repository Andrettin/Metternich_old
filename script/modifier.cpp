#include "script/modifier.h"

#include "database/gsml_property.h"
#include "script/modifier_effect/modifier_effect.h"

namespace metternich {

template <typename T>
modifier<T>::modifier()
{
}

template <typename T>
modifier<T>::~modifier()
{
}

template <typename T>
void modifier<T>::process_gsml_property(const gsml_property &property)
{
	std::unique_ptr<modifier_effect<T>> effect = modifier_effect<T>::from_gsml_property(property);
	this->add_modifier_effect(std::move(effect));
}

template <typename T>
void modifier<T>::apply(T *scope) const
{
	for (const std::unique_ptr<modifier_effect<T>> &modifier_effect : this->modifier_effects) {
		modifier_effect->apply(scope, 1);
	}
}

template <typename T>
void modifier<T>::remove(T *scope) const
{
	for (const std::unique_ptr<modifier_effect<T>> &modifier_effect : this->modifier_effects) {
		modifier_effect->apply(scope, -1);
	}
}

template <typename T>
std::string modifier<T>::get_string() const
{
	std::string str;
	for (size_t i = 0; i < this->modifier_effects.size(); ++i) {
		if (i > 0) {
			str += "\n";
		}
		str += this->modifier_effects[i]->get_string();
	}
	return str;
}

template <typename T>
void modifier<T>::add_modifier_effect(std::unique_ptr<modifier_effect<T>> &&modifier_effect)
{
	this->modifier_effects.push_back(std::move(modifier_effect));
}

template class modifier<character>;
template class modifier<holding>;
template class modifier<province>;

}
