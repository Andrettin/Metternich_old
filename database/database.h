#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace Metternich {

class DataEntry;

/**
**	@brief	The database
*/
class Database
{
public:
	static Database *GetInstance()
	{
		std::call_once(Database::OnceFlag, [](){ Database::Instance = std::make_unique<Database>(); });

		return Database::Instance.get();
	}

private:
	static inline std::unique_ptr<Database> Instance;
	static inline std::once_flag OnceFlag;

public:
	void Load()
	{
		//parse the files for in each data type's folder
		for (const std::function<void()> &function : this->ParsingFunctions) {
			function();
		}

		//create data entries for each data type
		for (const std::function<void(bool)> &function : this->ProcessingFunctions) {
			function(true);
		}

		//actually define the data entries for each data type
		for (const std::function<void(bool)> &function : this->ProcessingFunctions) {
			function(false);
		}

		//check if data entries are valid for each data type
		for (const std::function<void()> &function : this->CheckingFunctions) {
			function();
		}
	}

	void AddParsingFunction(const std::function<void()> &function)
	{
		this->ParsingFunctions.push_back(function);
	}

	void AddProcessingFunction(const std::function<void(bool)> &function)
	{
		this->ProcessingFunctions.push_back(function);
	}

	void AddCheckingFunction(const std::function<void()> &function)
	{
		this->CheckingFunctions.push_back(function);
	}

private:
	std::vector<std::function<void()>> ParsingFunctions; //parsing functions for each data type
	std::vector<std::function<void(bool)>> ProcessingFunctions; //processing functions for each data type
	std::vector<std::function<void()>> CheckingFunctions; //functions for each data type, to check if data entries are valid
};

}