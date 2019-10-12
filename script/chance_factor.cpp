#include "script/chance_factor.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "factor_modifier.h"
#include "util/util.h"

namespace metternich {

/**
**	@brief	Constructor
*/
ChanceFactor::ChanceFactor()
{
}

/**
**	@brief	Destructor
*/
ChanceFactor::~ChanceFactor()
{
}

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void ChanceFactor::process_gsml_property(const gsml_property &property)
{
	const std::string &key = property.get_key();
	const gsml_operator gsml_operator = property.get_operator();
	const std::string &value = property.get_value();

	if (key == "factor") {
		if (gsml_operator == gsml_operator::assignment) {
			this->Factor = util::centesimal_number_string_to_int(value);
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
void ChanceFactor::process_gsml_scope(const gsml_data &scope)
{
	if (scope.get_tag() == "modifier") {
		auto factor_modifier = std::make_unique<FactorModifier>();
		database::process_gsml_data(factor_modifier, scope);
		this->Modifiers.push_back(std::move(factor_modifier));
	} else {
		throw std::runtime_error("Invalid chance scope: " + scope.get_tag() + ".");
	}
}

}
