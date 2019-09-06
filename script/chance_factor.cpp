#include "script/chance_factor.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "factor_modifier.h"
#include "util.h"

namespace Metternich {

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
void ChanceFactor::ProcessGSMLProperty(const GSMLProperty &property)
{
	const std::string &key = property.GetKey();
	const GSMLOperator gsml_operator = property.GetOperator();
	const std::string &value = property.GetValue();

	if (key == "factor") {
		if (gsml_operator == GSMLOperator::Assignment) {
			this->Factor = CentesimalNumberStringToInt(value);
		} else {
			throw std::runtime_error("Invalid operator for property (\"" + property.GetKey() + "\").");
		}
	} else {
		throw std::runtime_error("Invalid chance factor property: \"" + property.GetKey() + "\".");
	}
}

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void ChanceFactor::ProcessGSMLScope(const gsml_data &scope)
{
	if (scope.get_tag() == "modifier") {
		auto factor_modifier = std::make_unique<FactorModifier>();
		Database::ProcessGSMLData(factor_modifier, scope);
		this->Modifiers.push_back(std::move(factor_modifier));
	} else {
		throw std::runtime_error("Invalid chance scope: " + scope.get_tag() + ".");
	}
}

}
