#include "script/condition/condition.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "script/condition/and_condition.h"
#include "script/condition/borders_water_condition.h"
#include "script/condition/commodity_condition.h"
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

	if (condition_identifier == "borders_water") {
		condition = std::make_unique<BordersWaterCondition>(StringToBool(property.GetValue()));
	} else if (condition_identifier == "commodity") {
		condition = std::make_unique<CommodityCondition>(property.GetValue());
	} else if (condition_identifier == "terrain") {
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
std::unique_ptr<Condition> Condition::FromGSMLScope(const gsml_data &scope)
{
	std::string condition_identifier = ToLower(scope.get_tag());
	std::unique_ptr<Condition> condition;

	if (condition_identifier == "and") {
		condition = std::make_unique<AndCondition>();
	} else if (condition_identifier == "or") {
		condition = std::make_unique<OrCondition>();
	} else if (condition_identifier == "not" || condition_identifier == "nor") {
		condition = std::make_unique<NotCondition>();
	} else if (condition_identifier == "nand") {
		auto and_condition = std::make_unique<AndCondition>();
		Database::ProcessGSMLData(and_condition, scope);
		condition = std::make_unique<NotCondition>(std::move(and_condition));
		return condition;
	} else {
		throw std::runtime_error("Invalid scope condition: \"" + condition_identifier + "\".");
	}

	Database::ProcessGSMLData(condition, scope);

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
void Condition::ProcessGSMLScope(const gsml_data &scope)
{
	throw std::runtime_error("Invalid \"" + this->GetIdentifier() + "\" condition scope: " + scope.get_tag() + ".");
}

}
