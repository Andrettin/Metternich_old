#include "factor_modifier.h"

#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "script/condition/condition.h"
#include "util.h"

namespace metternich {

/**
**	@brief	Constructor
*/
FactorModifier::FactorModifier()
{
}

/**
**	@brief	Destructor
*/
FactorModifier::~FactorModifier()
{
}

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void FactorModifier::ProcessGSMLProperty(const GSMLProperty &property)
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
		std::unique_ptr<Condition> condition = Condition::FromGSMLProperty(property);
		this->Conditions.push_back(std::move(condition));
	}
}

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void FactorModifier::ProcessGSMLScope(const gsml_data &scope)
{
	std::unique_ptr<Condition> condition = Condition::FromGSMLScope(scope);
	this->Conditions.push_back(std::move(condition));
}

}
