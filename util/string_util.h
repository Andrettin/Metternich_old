#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace metternich::util {

inline std::vector<std::string> split_string(const std::string &str, const char delimiter)
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
	} else if (str.substr(str.size() - 1, 1) == "s") {
		singular_form = str.substr(0, str.size() - 1);
	} else {
		return str;
	}

	return singular_form;
}

}
