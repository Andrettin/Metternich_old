#include "script/chance_factor.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_operator.h"
#include "database/gsml_property.h"
#include "factor_modifier.h"
#include "util/parse_util.h"

namespace metternich {

template <typename T>
chance_factor<T>::chance_factor()
{
}

template <typename T>
chance_factor<T>::chance_factor(const int factor) : factor(factor)
{
}

template <typename T>
chance_factor<T>::~chance_factor()
{
}

template <typename T>
void chance_factor<T>::process_gsml_property(const gsml_property &property)
{
	const std::string &key = property.get_key();
	const gsml_operator gsml_operator = property.get_operator();
	const std::string &value = property.get_value();

	if (key == "factor") {
		if (gsml_operator == gsml_operator::assignment) {
			this->factor = parse::centesimal_number_string_to_int(value);
		} else {
			throw std::runtime_error("Invalid operator for property (\"" + property.get_key() + "\").");
		}
	} else {
		throw std::runtime_error("Invalid chance factor property: \"" + property.get_key() + "\".");
	}
}

template <typename T>
void chance_factor<T>::process_gsml_scope(const gsml_data &scope)
{
	if (scope.get_tag() == "modifier") {
		auto modifier = std::make_unique<factor_modifier<T>>();
		database::process_gsml_data(modifier, scope);
		this->modifiers.push_back(std::move(modifier));
	} else {
		throw std::runtime_error("Invalid chance scope: " + scope.get_tag() + ".");
	}
}

template <typename T>
int chance_factor<T>::calculate(const T *scope, const read_only_context &ctx) const
{
	//get the resulting chance factor after taking into account all modifiers
	int result = this->factor;

	for (const std::unique_ptr<factor_modifier<T>> &modifier : this->modifiers) {
		if (modifier->check_conditions(scope, ctx)) {
			result *= modifier->get_factor();
			result /= 100;
		}
	}

	return result;
}

template class chance_factor<character>;
template class chance_factor<holding>;
template class chance_factor<holding_slot>;

}
