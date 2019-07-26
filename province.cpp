#include "province.h"

#include "csv_data.h"

void Province::ProcessDefinitions(const CSVData &csv_data)
{
	for (const std::vector<std::string> &line_values : csv_data.GetValues()) {
		if (line_values.empty()) {
			continue;
		}

		const int province_id = std::stoi(line_values.at(0));
		Province *province = Province::Add(province_id);

		if (line_values.size() < 4) {
			continue;
		}

		const int red = std::stoi(line_values.at(1));
		const int green = std::stoi(line_values.at(2));
		const int blue = std::stoi(line_values.at(3));
		province->Color.setRgb(red, green, blue);

		if (line_values.size() < 5) {
			continue;
		}

		province->Name = line_values.at(4);
	}
}
