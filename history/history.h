#pragma once

#include <QDateTime>
#include <QString>

#include <string>
#include <vector>

namespace metternich {

class History
{
public:
	static void Load();
	static void generate_population_units();

	static QDateTime StringToDate(const std::string &date_str);
};

}
