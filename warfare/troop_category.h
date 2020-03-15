#pragma once

namespace metternich {

enum class troop_category
{
	none,
	militia,
	light_infantry,
	regular_infantry,
	heavy_infantry,
	light_cavalry,
	heavy_cavalry,
	light_artillery,
	heavy_artillery,
	combat_engineers
};

inline troop_category string_to_troop_category(const std::string &str)
{
	if (str == "militia") {
		return troop_category::militia;
	} else if (str == "light_infantry") {
		return troop_category::light_infantry;
	} else if (str == "regular_infantry") {
		return troop_category::regular_infantry;
	} else if (str == "heavy_infantry") {
		return troop_category::heavy_infantry;
	} else if (str == "light_cavalry") {
		return troop_category::light_cavalry;
	} else if (str == "heavy_cavalry") {
		return troop_category::heavy_cavalry;
	} else if (str == "light_artillery") {
		return troop_category::light_artillery;
	} else if (str == "heavy_artillery") {
		return troop_category::heavy_artillery;
	} else if (str == "combat_engineers") {
		return troop_category::combat_engineers;
	}

	throw std::runtime_error("Invalid troop category: \"" + str + "\".");
}

}

Q_DECLARE_METATYPE(metternich::troop_category)
