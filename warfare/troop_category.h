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

inline std::string troop_category_to_string(const troop_category category)
{
	switch (category) {
		case troop_category::militia:
			return "militia";
		case troop_category::light_infantry:
			return "light_infantry";
		case troop_category::regular_infantry:
			return "regular_infantry";
		case troop_category::heavy_infantry:
			return "heavy_infantry";
		case troop_category::light_cavalry:
			return "light_cavalry";
		case troop_category::heavy_cavalry:
			return "heavy_cavalry";
		case troop_category::light_artillery:
			return "light_artillery";
		case troop_category::heavy_artillery:
			return "heavy_artillery";
		case troop_category::combat_engineers:
			return "combat_engineers";
		case troop_category::light_warship:
			return "light_warship";
		case troop_category::heavy_warship:
			return "heavy_warship";
		case troop_category::light_transport_ship:
			return "light_transport_ship";
		case troop_category::heavy_transport_ship:
			return "heavy_transport_ship";
		default:
			break;
	}

	throw std::runtime_error("Invalid troop category: \"" + std::to_string(static_cast<int>(category)) + "\".");
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
