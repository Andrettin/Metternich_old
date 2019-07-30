#pragma once

#include <QPoint>
#include <QSize>

#include <string>

namespace Metternich {

inline bool StringToBool(const std::string &str)
{
	if (str == "true" || str == "yes" || str == "1") {
		return true;
	} else if (str == "false" || str == "no" || str == "0") {
		return false;
	}

	throw std::runtime_error("Invalid string used for conversion to boolean: \"" + str + "\".");
}

inline std::vector<std::string> SplitString(const std::string &str, const char delimiter)
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

inline int PointToIndex(const QPoint &point, const QSize &size)
{
	return point.x() + point.y() * size.width();
}

inline QPoint IndexToPoint(const int index, const QSize &size)
{
	return QPoint(index % size.width(), index / size.width());
}

}
