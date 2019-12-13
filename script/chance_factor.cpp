#include "script/chance_factor.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_operator.h"
#include "database/gsml_property.h"
#include "factor_modifier.h"
#include "util/parse_util.h"

namespace metternich {

/**
**	@brief	Constructor
*/
template <typename T>
chance_factor<T>::chance_factor()
{
}

/**
**	@brief	Destructor
*/
template <typename T>
chance_factor<T>::~chance_factor()
{
}

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
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

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
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

template class chance_factor<holding_slot>;

}
