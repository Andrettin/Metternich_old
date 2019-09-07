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
void FactorModifier::ProcessGSMLProperty(const gsml_property &property)
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
