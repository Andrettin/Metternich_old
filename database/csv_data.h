#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace Metternich {

/**
**	@brief	CSV data
*/
class CSVData
{
public:
	static constexpr char Delimiter = ';';

	static CSVData ParseFile(const std::filesystem::path &filepath);

	const std::vector<std::vector<std::string>> &GetValues() const
	{
		return this->Values;
	}

private:
	std::vector<std::vector<std::string>> Values;
};

}
