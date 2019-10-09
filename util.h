#pragma once

#include <QCryptographicHash>
#include <QPoint>
#include <QSize>
#include <QVariant>

#include <algorithm>
#include <string>

namespace metternich::util {

/**
**	@brief	Convert a fractional number string (i.e. a number with up to two decimal places) to an integer, multiplying it by 10 ^ the number of decimal places; the template argument is the number of decimal places
**
**	@param	str	The string
**
**	@return	The integer
*/
template <int N>
inline int fractional_number_string_to_int(const std::string &str)
{
	size_t decimal_point_pos = str.find('.');
	int integer = 0;
	int fraction = 0;
	if (decimal_point_pos != std::string::npos) {
		integer = std::stoi(str.substr(0, decimal_point_pos));
		const size_t decimal_pos = decimal_point_pos + 1;
		const size_t decimal_places = str.length() - decimal_pos;
		fraction = std::stoi(str.substr(decimal_pos, decimal_places));
		if (decimal_places < N) {
			for (int i = static_cast<int>(decimal_places); i < N; ++i) {
				fraction *= 10;
			}
		}
		const bool negative = str.front() == '-';
		if (negative) {
			fraction *= -1;
		}
	} else {
		integer = std::stoi(str);
	}

	for (int i = 0; i < N; ++i) {
		integer *= 10;
	}
	integer += fraction;

	return integer;
}

inline int centesimal_number_string_to_int(const std::string &str)
{
	return fractional_number_string_to_int<2>(str);
}

/**
**	@brief	Round a double to a certain number of decimal places
**
**	@param	number	The number
**
**	@return	The rounded number
*/
template <int N>
inline double round_to_decimal_places(double number)
{
	for (int i = 0; i < N; ++i) {
		number *= 10.0;
	}

	number = std::round(number);

	for (int i = 0; i < N; ++i) {
		number /= 10.0;
	}

	return number;
}

inline double round_to_decimal(double number)
{
	return round_to_decimal_places<1>(number);
}

inline double round_to_centesimal(double number)
{
	return round_to_decimal_places<2>(number);
}

inline bool string_to_bool(const std::string &str)
{
	if (str == "true" || str == "yes" || str == "1") {
		return true;
	} else if (str == "false" || str == "no" || str == "0") {
		return false;
	}

	throw std::runtime_error("Invalid string used for conversion to boolean: \"" + str + "\".");
}

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

inline int point_to_index(const QPoint &point, const QSize &size)
{
	return point.x() + point.y() * size.width();
}

inline QPoint index_to_point(const int index, const QSize &size)
{
	return QPoint(index % size.width(), index / size.width());
}

template <typename T>
inline QVariantList container_to_qvariant_list(const T &container)
{
	QVariantList list;

	for (const typename T::value_type &element : container) {
		list.append(QVariant::fromValue(element));
	}

	return list;
}

template <typename T>
inline QObjectList vector_to_qobject_list(const std::vector<T *> &vector)
{
	QObjectList object_list;
	for (T *element : vector) {
		object_list.append(element);
	}
	return object_list;
}

/**
**	@brief	Add the files in a directory to a checksum
**
**	@param	hash		The checksum hash
**	@param	dir_path	The path to the directory
*/
inline void add_files_to_checksum(QCryptographicHash &hash, const std::filesystem::path &dir_path)
{
	std::filesystem::recursive_directory_iterator dir_iterator(dir_path);

	for (const std::filesystem::directory_entry &dir_entry : dir_iterator) {
		if (!dir_entry.is_regular_file()) {
			continue;
		}

		QFile file(QString::fromStdString(dir_entry.path().string()));

		if (!file.open(QIODevice::ReadOnly)) {
			throw std::runtime_error("Failed to open file: " + dir_entry.path().string() + ".");
		}

		if (!hash.addData(&file)) {
			throw std::runtime_error("Failed to hash file: " + dir_entry.path().string() + ".");
		}

		file.close();
	}
}

}
