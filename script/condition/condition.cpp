#include "script/condition/condition.h"

#include "character/character.h"
#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_operator.h"
#include "database/gsml_property.h"
#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "landed_title/landed_title_tier.h"
#include "map/province.h"
#include "population/population_unit.h"
#include "script/condition/alive_condition.h"
#include "script/condition/and_condition.h"
#include "script/condition/borders_water_condition.h"
#include "script/condition/commodity_condition.h"
#include "script/condition/culture_condition.h"
#include "script/condition/has_any_active_trade_route_condition.h"
#include "script/condition/has_any_trade_route_condition.h"
#include "script/condition/has_any_trade_route_land_connection_condition.h"
#include "script/condition/has_building_condition.h"
#include "script/condition/has_flag_condition.h"
#include "script/condition/has_item_condition.h"
#include "script/condition/has_law_condition.h"
#include "script/condition/has_technology_condition.h"
#include "script/condition/has_trait_condition.h"
#include "script/condition/holding_type_condition.h"
#include "script/condition/location_condition.h"
#include "script/condition/not_condition.h"
#include "script/condition/or_condition.h"
#include "script/condition/prowess_condition.h"
#include "script/condition/region_condition.h"
#include "script/condition/terrain_condition.h"
#include "script/condition/tier_de_jure_title_condition.h"
#include "script/condition/wealth_condition.h"
#include "script/condition/world_condition.h"
#include "util/parse_util.h"
#include "util/string_util.h"

namespace metternich {

template <typename T>
std::unique_ptr<condition<T>> condition<T>::from_gsml_property(const gsml_property &property)
{
	const std::string &condition_identifier = property.get_key();

	if constexpr (std::is_same_v<T, character>) {
		if (condition_identifier == "alive") {
			return std::make_unique<alive_condition<T>>(string::to_bool(property.get_value()), property.get_operator());
		} else if (condition_identifier == "has_item") {
			return std::make_unique<has_item_condition<T>>(property.get_value(), property.get_operator());
		} else if (condition_identifier == "has_law") {
			return std::make_unique<has_law_condition<T>>(property.get_value(), property.get_operator());
		} else if (condition_identifier == "has_trait") {
			return std::make_unique<has_trait_condition<T>>(property.get_value(), property.get_operator());
		} else if (condition_identifier == "prowess") {
			return std::make_unique<prowess_condition<T>>(std::stoi(property.get_value()), property.get_operator());
		} else if (condition_identifier == "wealth") {
			return std::make_unique<wealth_condition<T>>(property.get_value(), property.get_operator());
		}
	} else {
		if (condition_identifier == "borders_water") {
			return std::make_unique<borders_water_condition<T>>(string::to_bool(property.get_value()), property.get_operator());
		} else if (condition_identifier == "de_jure_duchy") {
			return std::make_unique<tier_de_jure_title_condition<T, landed_title_tier::duchy>>(property.get_value(), property.get_operator());
		} else if (condition_identifier == "de_jure_kingdom") {
			return std::make_unique<tier_de_jure_title_condition<T, landed_title_tier::kingdom>>(property.get_value(), property.get_operator());
		} else if (condition_identifier == "de_jure_empire") {
			return std::make_unique<tier_de_jure_title_condition<T, landed_title_tier::empire>>(property.get_value(), property.get_operator());
		} else if (condition_identifier == "has_technology") {
			return std::make_unique<has_technology_condition<T>>(property.get_value(), property.get_operator());
		} else if (condition_identifier == "region") {
			return std::make_unique<region_condition<T>>(property.get_value(), property.get_operator());
		} else if (condition_identifier == "terrain") {
			return std::make_unique<terrain_condition<T>>(property.get_value(), property.get_operator());
		} else if (condition_identifier == "world") {
			return std::make_unique<world_condition<T>>(property.get_value(), property.get_operator());
		}
	}

	if constexpr (std::is_same_v<T, holding>) {
		if (condition_identifier == "commodity") {
			return std::make_unique<commodity_condition<T>>(property.get_value(), property.get_operator());
		} else if (condition_identifier == "has_building") {
			return std::make_unique<has_building_condition<T>>(property.get_value(), property.get_operator());
		} else if (condition_identifier == "holding_type") {
			return std::make_unique<holding_type_condition<T>>(property.get_value(), property.get_operator());
		}
	}

	if constexpr (!std::is_same_v<T, holding_slot>) {
		if (condition_identifier == "culture") {
			return std::make_unique<culture_condition<T>>(property.get_value(), property.get_operator());
		}
	}

	if constexpr (std::is_same_v<T, holding> || std::is_same_v<T, holding_slot> || std::is_same_v<T, province>) {
		if (condition_identifier == "has_any_active_trade_route") {
			return std::make_unique<has_any_active_trade_route_condition<T>>(string::to_bool(property.get_value()), property.get_operator());
		} else if (condition_identifier == "has_any_trade_route") {
			return std::make_unique<has_any_trade_route_condition<T>>(string::to_bool(property.get_value()), property.get_operator());
		} else if (condition_identifier == "has_any_trade_route_land_connection") {
			return std::make_unique<has_any_trade_route_land_connection_condition<T>>(string::to_bool(property.get_value()), property.get_operator());
		}
	}

	if constexpr (std::is_same_v<T, character>) {
		if (condition_identifier == "has_flag") {
			return std::make_unique<has_flag_condition<T>>(property.get_value(), property.get_operator());
		}
	}

	throw std::runtime_error("Invalid property condition: \"" + condition_identifier + "\".");
}

template <typename T>
std::unique_ptr<condition<T>> condition<T>::from_gsml_scope(const gsml_data &scope)
{
	std::string condition_identifier = string::to_lower(scope.get_tag());
	std::unique_ptr<condition> condition;

	if (condition_identifier == "and") {
		condition = std::make_unique<and_condition<T>>(scope.get_operator());
	} else if (condition_identifier == "or") {
		condition = std::make_unique<or_condition<T>>(scope.get_operator());
	} else if (condition_identifier == "not" || condition_identifier == "nor") {
		condition = std::make_unique<not_condition<T>>(scope.get_operator());
	} else if (condition_identifier == "nand") {
		auto and_condition = std::make_unique<metternich::and_condition<T>>();
		database::process_gsml_data(and_condition, scope);
		condition = std::make_unique<not_condition<T>>(std::move(and_condition), scope.get_operator());
		return condition;
	} else {
		if constexpr (std::is_same_v<T, character>) {
			if (condition_identifier == "location") {
				condition = std::make_unique<location_condition<T>>(scope.get_operator());
			}
		}
	}

	if (condition == nullptr) {
		throw std::runtime_error("Invalid scope condition: \"" + condition_identifier + "\".");
	}

	database::process_gsml_data(condition, scope);

	return condition;
}

template <typename T>
void condition<T>::process_gsml_property(const gsml_property &property)
{
	throw std::runtime_error("Invalid " + this->get_identifier() + " condition property: " + property.get_key() + ".");
}

template <typename T>
void condition<T>::process_gsml_scope(const gsml_data &scope)
{
	throw std::runtime_error("Invalid " + this->get_identifier() + " condition scope: " + scope.get_tag() + ".");
}

template <typename T>
bool condition<T>::check(const T *scope, const read_only_context &ctx) const
{
	switch (this->get_operator()) {
		case gsml_operator::assignment:
			return this->check_assignment(scope, ctx);
		case gsml_operator::equality:
			return this->check_equality(scope);
		case gsml_operator::inequality:
			return this->check_inequality(scope);
		case gsml_operator::less_than:
			return this->check_less_than(scope);
		case gsml_operator::less_than_or_equality:
			return this->check_less_than_or_equality(scope);
		case gsml_operator::greater_than:
			return this->check_greater_than(scope);
		case gsml_operator::greater_than_or_equality:
			return this->check_greater_than_or_equality(scope);
		default:
			throw std::runtime_error("Invalid condition operator: \"" + std::to_string(static_cast<int>(this->get_operator())) + "\".");
	}
}


template <typename T>
std::string condition<T>::get_string(const T *scope, const read_only_context &ctx, const size_t indent) const
{
	std::string str = "(";

	if (this->check(scope, ctx)) {
		str += string::color("*", "green");
	} else {
		str += string::color("x", "red");
	}

	str += ") ";

	switch (this->get_operator()) {
		case gsml_operator::assignment:
			str += this->get_assignment_string(scope, ctx, indent);
			break;
		case gsml_operator::equality:
			str += this->get_equality_string();
			break;
		case gsml_operator::inequality:
			str += this->get_inequality_string();
			break;
		case gsml_operator::less_than:
			str += this->get_less_than_string();
			break;
		case gsml_operator::less_than_or_equality:
			str += this->get_less_than_or_equality_string();
			break;
		case gsml_operator::greater_than:
			str += this->get_greater_than_string();
			break;
		case gsml_operator::greater_than_or_equality:
			str += this->get_greater_than_or_equality_string();
			break;
		default:
			throw std::runtime_error("Invalid condition operator: \"" + std::to_string(static_cast<int>(this->get_operator())) + "\".");
	}

	return str;
}

template class condition<character>;
template class condition<holding>;
template class condition<holding_slot>;
template class condition<population_unit>;
template class condition<province>;

}
