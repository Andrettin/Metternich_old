#pragma once

#include "database/gsml_data.h"
#include "singleton.h"
#include "type_traits.h"

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace metternich {

class data_entry;

/**
**	@brief	The database
*/
class Database : public Singleton<Database>
{
public:
	template <typename T>
	static void ProcessGSMLData(T *instance, const gsml_data &data)
	{
		for (const gsml_property &property : data.get_properties()) {
			instance->process_gsml_property(property);
		}

		for (const gsml_data &child_data : data.get_children()) {
			instance->process_gsml_scope(child_data);
		}
	}

	template <typename T>
	static void ProcessGSMLData(T &instance, const gsml_data &data)
	{
		if constexpr (is_specialization_of_v<T, std::unique_ptr>) {
			Database::ProcessGSMLData(instance.get(), data);
		} else {
			Database::ProcessGSMLData(&instance, data);
		}
	}

	static void ProcessGSMLPropertyForObject(QObject *object, const gsml_property &property);

public:
	void Load();

	void initialize_history()
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
