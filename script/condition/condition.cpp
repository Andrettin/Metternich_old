#include "script/condition/condition.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "script/condition/and_condition.h"
#include "script/condition/not_condition.h"
#include "script/condition/or_condition.h"
#include "script/condition/terrain_condition.h"
#include "util.h"

namespace Metternich {

/**
**	@brief	Create a condition from a GSML property
**
**	@param	gsml_property	The GSML property
*/
std::unique_ptr<Condition> Condition::FromGSMLProperty(const GSMLProperty &property)
{
	std::string condition_identifier = ToLower(property.GetKey());
	std::unique_ptr<Condition> condition;

	if (condition_identifier == "terrain") {
		condition = std::make_unique<TerrainCondition>(property.GetValue());
	} else {
		throw std::runtime_error("Invalid property condition: \"" + condition_identifier + "\".");
	}

	return condition;
}

/**
**	@brief	Create a condition from a GSML scope
**
**	@param	scope	The GSML scope
*/
std::unique_ptr<Condition> Condition::FromGSMLScope(const GSMLData &scope)
{
	std::string condition_identifier = ToLower(scope.GetTag());
	std::unique_ptr<Condition> condition;

	if (condition_identifier == "and") {
		condition = std::make_unique<AndCondition>();
	} else if (condition_identifier == "or") {
		condition = std::make_unique<OrCondition>();
	} else if (condition_identifier == "not") {
		condition = std::make_unique<NotCondition>();
	} else {
		throw std::runtime_error("Invalid scope condition: \"" + condition_identifier + "\".");
	}

	Database::ProcessGSMLData(condition.get(), scope);

	return condition;
}

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void Condition::ProcessGSMLProperty(const GSMLProperty &property)
{
	throw std::runtime_error("Invalid \"" + this->GetIdentifier() + "\" condition property: " + property.GetKey() + ".");
}

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void Condition::ProcessGSMLScope(const GSMLData &scope)
{
	throw std::runtime_error("Invalid \"" + this->GetIdentifier() + "\" condition scope: " + scope.GetTag() + ".");
}

}
