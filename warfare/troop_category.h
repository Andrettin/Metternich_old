#pragma once

namespace metternich {

enum class troop_category
{
	infantry,
	cavalry,
	artillery
};

inline troop_category string_to_troop_category(const std::string &str)
{
	if (str == "infantry") {
		return troop_category::infantry;
	} else if (str == "cavalry") {
		return troop_category::cavalry;
	} else if (str == "artillery") {
		return troop_category::artillery;
	}

	throw std::runtime_error("Invalid troop category: \"" + str + "\".");
}

}

Q_DECLARE_METATYPE(metternich::troop_category)
