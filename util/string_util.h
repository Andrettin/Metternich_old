#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace metternich::string {

inline bool to_bool(const std::string &str)
{
	if (str == "true" || str == "yes" || str == "1") {
		return true;
	} else if (str == "false" || str == "no" || str == "0") {
		return false;
	}

	throw std::runtime_error("Invalid string used for conversion to boolean: \"" + str + "\".");
}

inline std::vector<std::string> split(const std::string &str, const char delimiter)
{
	std::vector<std::string> string_list;

	size_t start_pos = 0;
	size_t find_pos = 0;
	while ((find_pos = str.find(delimiter, start_pos)) != std::string::npos) {
		std::string string_element = str.substr(start_pos, find_pos - start_pos);
		string_list.push_back(string_element);
		start_pos = find_pos + 1;
	}

	std::string string_element = str.substr(start_pos, str.length() - start_pos);
	string_list.push_back(string_element);

	return string_list;
}

inline std::string to_lower(const std::string &str)
{
	std::string result(str);

	std::transform(result.begin(), result.end(), result.begin(), [](const char c) {
		return std::tolower(c);
	});

	return result;
}

inline std::string replace(const std::string &str, const std::string &find, const std::string &replace)
{
	std::string result(str);

	size_t pos = 0;
	while ((pos = result.find(find, pos)) != std::string::npos) {
		result.replace(pos, find.length(), replace);
		pos += replace.length();
	}

	return result;
}

inline int get_substring_count(const std::string &str, const std::string &find)
{
	int count = 0;

	size_t pos = 0;
	while ((pos = str.find(find, pos)) != std::string::npos) {
		count++;
		pos += find.size();
	}

	return count;
}

inline std::string pascal_case_to_snake_case(const std::string &str)
{
	if (str.empty()) {
		return str;
	}

	std::string result(str);

	result[0] = static_cast<char>(tolower(result[0]));

	for (size_t i = 1; i < result.size(); ++i) {
		if (isupper(result[i])) {
			std::string replacement;
			replacement += '_';
			replacement += static_cast<char>(tolower(result[i]));
			result.replace(i, 1, replacement);
			++i; //because of the underline
		}
	}

	return result;
}

inline std::string snake_case_to_pascal_case(const std::string &str)
{
	if (str.empty()) {
		return str;
	}

	std::string result;
	result += static_cast<char>(toupper(str[0]));

	for (size_t pos = 1; pos < str.length(); ++pos) {
		if (str[pos] == '_') {
			++pos;
			result += static_cast<char>(toupper(str[pos]));
		} else {
			result += str[pos];
		}
	}

	return result;
}

inline std::string get_singular_form(const std::string &str)
{
	std::string singular_form;

	if (str.substr(str.size() - 2, 2) == "ys") {
		singular_form = str.substr(0, str.size() - 2);
	} else if (str.substr(str.size() - 3, 3) == "ies") {
		singular_form = str.substr(0, str.size() - 3) + "y";
	} else if (str.substr(str.size() - 1, 1) == "s") {
		singular_form = str.substr(0, str.size() - 1);
	} else {
		return str;
	}

	return singular_form;
}

inline std::vector<std::string> get_suffix_combinations(const std::vector<std::vector<std::string>> &suffix_list_with_fallbacks)
{
	std::vector<std::string> suffix_combinations; //possible combinations of tags/suffixes, from more specific to less specific

	for (const std::vector<std::string> &suffix_with_fallbacks : suffix_list_with_fallbacks) {
		unsigned int added_suffixes = 0;
		for (unsigned int i = 0; i < suffix_combinations.size(); i += (1 + added_suffixes)) {
			added_suffixes = 0;
			for (const std::string &suffix_tag : suffix_with_fallbacks) {
				suffix_combinations.insert(suffix_combinations.begin() + i + added_suffixes, suffix_combinations[i] + "_" + suffix_tag);
				added_suffixes++;
			}
		}

		for (const std::string &suffix_tag : suffix_with_fallbacks) {
			suffix_combinations.push_back("_" + suffix_tag);
		}
	}

	suffix_combinations.push_back(""); //no suffix

	return suffix_combinations;
}

inline std::string highlight(const std::string &str)
{
	return "<font color=\"gold\">" + str + "</font>";
}

}
