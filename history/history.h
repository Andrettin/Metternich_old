#pragma once

#include <QDateTime>
#include <QString>

#include <string>
#include <vector>

namespace metternich {

class history
{
public:
	static void load();
	static void generate_population_units();

	static QDateTime string_to_date(const std::string &date_str);
};

}
