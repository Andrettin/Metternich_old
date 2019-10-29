#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace metternich {

/**
**	@brief	CSV data
*/
class csv_data
{
public:
	static constexpr char delimiter = ';';

	static csv_data parse_file(const std::filesystem::path &filepath);

	const std::vector<std::vector<std::string>> &get_values() const
	{
		return this->values;
	}

private:
	std::vector<std::vector<std::string>> values;
};

}
