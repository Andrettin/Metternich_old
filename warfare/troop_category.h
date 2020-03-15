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
	combat_engineers,
	light_warship,
	heavy_warship,
	light_transport_ship,
	heavy_transport_ship
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
	} else if (str == "light_warship") {
		return troop_category::light_warship;
	} else if (str == "heavy_warship") {
		return troop_category::heavy_warship;
	} else if (str == "light_transport_ship") {
		return troop_category::light_transport_ship;
	} else if (str == "heavy_transport_ship") {
		return troop_category::heavy_transport_ship;
	}

	throw std::runtime_error("Invalid troop category: \"" + str + "\".");
}

inline bool is_ship_troop_category(const troop_category category)
{
	switch (category) {
		case troop_category::light_warship:
		case troop_category::heavy_warship:
		case troop_category::light_transport_ship:
		case troop_category::heavy_transport_ship:
			return true;
		default:
			return false;
	}
}

}

Q_DECLARE_METATYPE(metternich::troop_category)
