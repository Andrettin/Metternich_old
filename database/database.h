#pragma once

#include "database/gsml_data.h"
#include "type_traits.h"

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

	template <typename T>
	static void ProcessGSMLData(T *instance, const GSMLData &gsml_data)
	{
		for (const GSMLProperty &property : gsml_data.GetProperties()) {
			instance->ProcessGSMLProperty(property);
		}

		for (const GSMLData &child_data : gsml_data.GetChildren()) {
			instance->ProcessGSMLScope(child_data);
		}
	}

	template <typename T>
	static void ProcessGSMLData(T &instance, const GSMLData &gsml_data)
	{
		if constexpr (is_specialization_of_v<T, std::unique_ptr>) {
			Database::ProcessGSMLData(instance.get(), gsml_data);
		} else {
			Database::ProcessGSMLData(&instance, gsml_data);
		}
	}

	static void ProcessGSMLPropertyForObject(QObject *object, const GSMLProperty &property);

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

		//initialize data entries for each data type
		for (const std::function<void()> &function : this->InitializationFunctions) {
			function();
		}

		//check if data entries are valid for each data type
		for (const std::function<void()> &function : this->CheckingFunctions) {
			function();
		}
	}

	void InitializeHistory()
	{
		//initialize data entries are valid for each data type
		for (const std::function<void()> &function : this->HistoryInitializationFunctions) {
			function();
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

	void AddInitializationFunction(const std::function<void()> &function)
	{
		this->InitializationFunctions.push_back(function);
	}

	void AddHistoryInitializationFunction(const std::function<void()> &function)
	{
		this->HistoryInitializationFunctions.push_back(function);
	}

private:
	std::vector<std::function<void()>> ParsingFunctions; //parsing functions for each data type
	std::vector<std::function<void(bool)>> ProcessingFunctions; //processing functions for each data type
	std::vector<std::function<void()>> CheckingFunctions; //functions for each data type, to check if data entries are valid
	std::vector<std::function<void()>> InitializationFunctions; //functions for each data type, to initialize their entries
	std::vector<std::function<void()>> HistoryInitializationFunctions; //functions for each data type, to initialize their entries' history
};

}
