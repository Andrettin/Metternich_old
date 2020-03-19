#pragma once

#include <string>

namespace metternich {

enum class technology_category
{
	none,
	army,
	commerce,
	culture,
	industry,
	navy
};

inline technology_category string_to_technology_category(const std::string &str)
{
	if (str == "army") {
		return technology_category::army;
	} else if (str == "commerce") {
		return technology_category::commerce;
	} else if (str == "culture") {
		return technology_category::culture;
	} else if (str == "industry") {
		return technology_category::industry;
	} else if (str == "navy") {
		return technology_category::navy;
	}

	throw std::runtime_error("Invalid technology category: \"" + str + "\".");
}

inline std::string technology_category_to_string(const technology_category category)
{
	switch (category) {
		case technology_category::army:
			return "army";
		case technology_category::commerce:
			return "commerce";
		case technology_category::culture:
			return "culture";
		case technology_category::industry:
			return "industry";
		case technology_category::navy:
			return "navy";
		default:
			break;
	}

	throw std::runtime_error("Invalid technology category: \"" + std::to_string(static_cast<int>(category)) + "\".");
}

}

Q_DECLARE_METATYPE(metternich::technology_category)
