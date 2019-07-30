#pragma once

#include <QDateTime>
#include <QString>

#include <string>

namespace Metternich {

class History
{
public:
	static constexpr const char *DateFormat = "yyyy.M.d";
	static constexpr const char *DateTimeFormat = "yyyy.M.d.H";

	static void Load();

	static inline QDateTime StringToDate(const std::string &date_str)
	{
		QString date_qstr = QString::fromStdString(date_str);
		QDateTime date;
		if (date_qstr.count(".") == 3) {
			date = QDateTime::fromString(date_qstr, History::DateTimeFormat);
		} else {
			date = QDateTime::fromString(date_qstr, History::DateFormat);
		}

		if (!date.isValid()) {
			throw std::runtime_error("Date \"" + date_str + "\" is not a valid date!");
		}

		date.setTimeSpec(Qt::UTC);

		return date;
	}
};

}
