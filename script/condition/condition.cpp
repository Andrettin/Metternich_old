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

namespace metternich {

/**
**	@brief	Create a condition from a GSML property
**
**	@param	gsml_property	The GSML property
*/
std::unique_ptr<Condition> Condition::FromGSMLProperty(const gsml_property &property)
{
	std::string condition_identifier = util::to_lower(property.get_key());
	std::unique_ptr<Condition> condition;

	if (condition_identifier == "borders_water") {
		condition = std::make_unique<BordersWaterCondition>(util::string_to_bool(property.get_value()));
	} else if (condition_identifier == "commodity") {
		condition = std::make_unique<commodity_condition>(property.get_value());
	} else if (condition_identifier == "terrain") {
		condition = std::make_unique<TerrainCondition>(property.get_value());
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
	std::string condition_identifier = util::to_lower(scope.get_tag());
	std::unique_ptr<Condition> condition;

	if (condition_identifier == "and") {
		condition = std::make_unique<AndCondition>();
	} else if (condition_identifier == "or") {
		condition = std::make_unique<OrCondition>();
	} else if (condition_identifier == "not" || condition_identifier == "nor") {
		condition = std::make_unique<NotCondition>();
	} else if (condition_identifier == "nand") {
		auto and_condition = std::make_unique<AndCondition>();
		database::process_gsml_data(and_condition, scope);
		condition = std::make_unique<NotCondition>(std::move(and_condition));
		return condition;
	} else {
		throw std::runtime_error("Invalid scope condition: \"" + condition_identifier + "\".");
	}

	database::process_gsml_data(condition, scope);

	return condition;
}

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void Condition::process_gsml_property(const gsml_property &property)
{
	throw std::runtime_error("Invalid \"" + this->get_identifier() + "\" condition property: " + property.get_key() + ".");
}

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void Condition::process_gsml_scope(const gsml_data &scope)
{
	throw std::runtime_error("Invalid \"" + this->get_identifier() + "\" condition scope: " + scope.get_tag() + ".");
}

}
