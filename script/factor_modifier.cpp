#include "factor_modifier.h"

#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "script/condition/condition.h"
#include "util/parse_util.h"

namespace metternich {

/**
**	@brief	Constructor
*/
factor_modifier::factor_modifier()
{
}

/**
**	@brief	Destructor
*/
factor_modifier::~factor_modifier()
{
}

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void factor_modifier::process_gsml_property(const gsml_property &property)
{
	const std::string &key = property.get_key();
	const gsml_operator gsml_operator = property.get_operator();
	const std::string &value = property.get_value();

	if (key == "factor") {
		if (gsml_operator == gsml_operator::assignment) {
			this->factor = util::centesimal_number_string_to_int(value);
		} else {
			throw std::runtime_error("Invalid operator for property (\"" + property.get_key() + "\").");
		}
	} else {
		std::unique_ptr<condition> condition = condition::from_gsml_property(property);
		this->conditions.push_back(std::move(condition));
	}
}

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void factor_modifier::process_gsml_scope(const gsml_data &scope)
{
	std::unique_ptr<condition> condition = condition::from_gsml_scope(scope);
	this->conditions.push_back(std::move(condition));
}

}
