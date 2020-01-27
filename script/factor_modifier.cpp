#include "factor_modifier.h"

#include "database/gsml_data.h"
#include "database/gsml_operator.h"
#include "database/gsml_property.h"
#include "script/condition/condition.h"
#include "util/parse_util.h"

namespace metternich {

template <typename T>
factor_modifier<T>::factor_modifier()
{
}

template <typename T>
factor_modifier<T>::~factor_modifier()
{
}

template <typename T>
void factor_modifier<T>::process_gsml_property(const gsml_property &property)
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
		std::unique_ptr<condition<T>> condition = metternich::condition<T>::from_gsml_property(property);
		this->conditions.push_back(std::move(condition));
	}
}

template <typename T>
void factor_modifier<T>::process_gsml_scope(const gsml_data &scope)
{
	std::unique_ptr<condition<T>> condition = metternich::condition<T>::from_gsml_scope(scope);
	this->conditions.push_back(std::move(condition));
}

template class factor_modifier<character>;
template class factor_modifier<holding>;
template class factor_modifier<holding_slot>;
template class factor_modifier<province>;

}
