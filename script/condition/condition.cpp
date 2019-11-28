#include "script/condition/condition.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "map/province.h"
#include "script/condition/and_condition.h"
#include "script/condition/borders_water_condition.h"
#include "script/condition/commodity_condition.h"
#include "script/condition/has_building_condition.h"
#include "script/condition/holding_type_condition.h"
#include "script/condition/not_condition.h"
#include "script/condition/or_condition.h"
#include "script/condition/region_condition.h"
#include "script/condition/terrain_condition.h"
#include "script/condition/world_condition.h"
#include "util/parse_util.h"
#include "util/string_util.h"

namespace metternich {

/**
**	@brief	Create a condition from a GSML property
**
**	@param	gsml_property	The GSML property
*/
template <typename T>
std::unique_ptr<condition<T>> condition<T>::from_gsml_property(const gsml_property &property)
{
	std::string condition_identifier = util::to_lower(property.get_key());
	std::unique_ptr<condition> condition;

	if (condition_identifier == "borders_water") {
		condition = std::make_unique<borders_water_condition<T>>(util::string_to_bool(property.get_value()));
	} else if (condition_identifier == "region") {
		condition = std::make_unique<region_condition<T>>(property.get_value());
	} else if (condition_identifier == "terrain") {
		condition = std::make_unique<terrain_condition<T>>(property.get_value());
	} else if (condition_identifier == "world") {
		condition = std::make_unique<world_condition<T>>(property.get_value());
	}

	if constexpr (std::is_same_v<T, holding>) {
		if (condition_identifier == "commodity") {
			condition = std::make_unique<commodity_condition<T>>(property.get_value());
		} else if (condition_identifier == "has_building") {
			condition = std::make_unique<has_building_condition<T>>(property.get_value());
		} else if (condition_identifier == "holding_type") {
			condition = std::make_unique<holding_type_condition<T>>(property.get_value());
		}
	}

	if (condition == nullptr) {
		throw std::runtime_error("Invalid property condition: \"" + condition_identifier + "\".");
	}

	return condition;
}

/**
**	@brief	Create a condition from a GSML scope
**
**	@param	scope	The GSML scope
*/
template <typename T>
std::unique_ptr<condition<T>> condition<T>::from_gsml_scope(const gsml_data &scope)
{
	std::string condition_identifier = util::to_lower(scope.get_tag());
	std::unique_ptr<condition> condition;

	if (condition_identifier == "and") {
		condition = std::make_unique<and_condition<T>>();
	} else if (condition_identifier == "or") {
		condition = std::make_unique<or_condition<T>>();
	} else if (condition_identifier == "not" || condition_identifier == "nor") {
		condition = std::make_unique<not_condition<T>>();
	} else if (condition_identifier == "nand") {
		auto and_condition = std::make_unique<metternich::and_condition<T>>();
		database::process_gsml_data(and_condition, scope);
		condition = std::make_unique<not_condition<T>>(std::move(and_condition));
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
template <typename T>
void condition<T>::process_gsml_property(const gsml_property &property)
{
	throw std::runtime_error("Invalid \"" + this->get_identifier() + "\" condition property: " + property.get_key() + ".");
}

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
template <typename T>
void condition<T>::process_gsml_scope(const gsml_data &scope)
{
	throw std::runtime_error("Invalid \"" + this->get_identifier() + "\" condition scope: " + scope.get_tag() + ".");
}

template class condition<holding>;
template class condition<holding_slot>;
template class condition<province>;

}
