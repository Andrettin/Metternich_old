#pragma once

#include <QDateTime>
#include <QString>

#include <string>
#include <vector>

namespace Metternich {

class Holding;

class History
{
public:
	static void Load();
	static void GeneratePopulationUnits();

	static QDateTime StringToDate(const std::string &date_str);
};

}
