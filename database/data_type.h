#pragma once

#include "database/database.h"
#include "database/gsml_data.h"

#include <filesystem>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace Metternich {

template <typename T, typename KEY = std::string>
class DataType
{
public:
	static constexpr bool HistoryOnly = false; //whether the data type is defined in history only

	/**
	**	@brief	Get an instance of the class by its identifier
	**
	**	@param	identifier	The instance's identifier
	**
	**	@param	should_find	Whether it is expected that an instance should be found (i.e. if none is, then it is an error).
	**
	**	@return	The instance if found, or null otherwise
	*/
	static T *Get(const KEY &identifier, const bool should_find = true)
	{
		typename std::map<KEY, std::unique_ptr<T>>::const_iterator find_iterator = DataType::InstancesByIdentifier.find(identifier);

		if (find_iterator != DataType::InstancesByIdentifier.end()) {
			return find_iterator->second.get();
		}

		if (should_find) {
			if constexpr (std::is_arithmetic_v<KEY>) {
				throw std::runtime_error("Invalid \"" + std::string(T::ClassIdentifier) + "\" instance: \"" + std::to_string(identifier) + "\".");
			} else {
				throw std::runtime_error("Invalid \"" + std::string(T::ClassIdentifier) + "\" instance: \"" + identifier + "\".");
			}
		}

		return nullptr;
	}

	/**
	**	@brief	Gets all instances of the class
	**
	**	@return	All existing instances of the class
	*/
	static const std::vector<T *> &GetAll()
	{
		return DataType::Instances;
	}

	/**
	**	@brief	Add a new instance of the class
	**
	**	@param	identifier	The instance's identifier
	**
	**	@return	The new instance
	*/
	static T *Add(const KEY &identifier)
	{
		if constexpr (std::is_same_v<KEY, std::string>) {
			if (identifier.empty()) {
				throw std::runtime_error("Tried to add a \"" + std::string(T::ClassIdentifier) + "\" instance with an empty string identifier.");
			}
		}

		DataType::InstancesByIdentifier[identifier] = std::make_unique<T>(identifier);
		T *instance = DataType::InstancesByIdentifier.find(identifier)->second.get();
		DataType::Instances.push_back(instance);

		if constexpr (std::is_same_v<KEY, int>) {
			if (identifier > DataType::LastNumericIdentifier) {
				DataType::LastNumericIdentifier = identifier;
			}
		}

		return instance;
	}

	/**
	**	@brief	Remove an instance of the class
	**
	**	@param	instance	The instance
	*/
	static void Remove(T *instance)
	{
		DataType::InstancesByIdentifier.erase(instance->GetIdentifier());
		DataType::Instances.erase(std::remove(DataType::Instances.begin(), DataType::Instances.end(), instance), DataType::Instances.end());
	}

	/**
	**	@brief	Remove the existing class instances
	*/
	static void Clear()
	{
		DataType::Instances.clear();
		DataType::InstancesByIdentifier.clear();
	}

	/**
	**	@brief	Parse the database for the data type
	*/
	static void ParseDatabase()
	{
		std::filesystem::path database_path("./data/common/" + std::string(T::DatabaseFolder));

		if (!std::filesystem::exists(database_path)) {
			return;
		}

		std::filesystem::recursive_directory_iterator dir_iterator(database_path);

		for (const std::filesystem::directory_entry &dir_entry : dir_iterator) {
			if (!dir_entry.is_regular_file()) {
				continue;
			}

			T::GSMLDataToProcess.push_back(GSMLData::ParseFile(dir_entry.path()));
		}
	}

	/**
	**	@brief	Process the database for the data type
	**
	**	@param	definition	Whether data entries are only being defined, or if their properties are actually being processed.
	*/
	static void ProcessDatabase(const bool definition)
	{
		for (const GSMLData &gsml_data : T::GSMLDataToProcess) {
			for (const GSMLData &data_entry : gsml_data.GetChildren()) {
				KEY identifier;
				if constexpr (std::is_same_v<KEY, int>) {
					identifier = std::stoi(data_entry.GetTag());
				} else {
					identifier = data_entry.GetTag();
				}

				T *instance = nullptr;
				if (definition) {
					instance = T::Add(identifier);
				} else {
					instance = T::Get(identifier);

					for (const GSMLProperty &property : data_entry.GetProperties()) {
						instance->ProcessGSMLProperty(property);
					}

					for (const GSMLData &child_data : data_entry.GetChildren()) {
						instance->ProcessGSMLScope(child_data);
					}
				}
			}
		}

		if (!definition) {
			T::GSMLDataToProcess.clear();
		}
	}

	static int GenerateNumericIdentifier()
	{
		return ++DataType::LastNumericIdentifier;
	}

	/**
	**	@brief	Parse the history database for the class
	*/
	static void ParseHistoryDatabase()
	{
		std::filesystem::path history_path("./data/history/" + std::string(T::DatabaseFolder));

		if (!std::filesystem::exists(history_path)) {
			return;
		}

		//non-history only data types have files with the same name as their identifiers, while for history only data types the file name is not relevant, with the identifier being scoped to within a file
		if constexpr (T::HistoryOnly == false) {
			for (T *instance : T::GetAll()) {
				std::filesystem::path history_file_path(history_path.string() + "/" + instance->GetIdentifier() + ".txt");

				if (!std::filesystem::exists(history_file_path)) {
					continue;
				}

				T::GSMLHistoryDataToProcess.push_back(GSMLData::ParseFile(history_file_path));
			}
		} else {
			std::filesystem::recursive_directory_iterator dir_iterator(history_path);

			for (const std::filesystem::directory_entry &dir_entry : dir_iterator) {
				if (!dir_entry.is_regular_file()) {
					continue;
				}

				T::GSMLHistoryDataToProcess.push_back(GSMLData::ParseFile(dir_entry.path()));
			}
		}
	}

	/**
	**	@brief	Process the history database for the class
	**
	**	@param	definition	Whether data entries are only being defined, or if their properties are actually being processed.
	*/
	static void ProcessHistoryDatabase(const bool definition)
	{
		//non-history only data types have files with the same name as their identifiers, while for history only data types the file name is not relevant, with the identifier being scoped to within a file
		if constexpr (T::HistoryOnly == false) {
			if (definition) {
				return;
			}

			for (GSMLData &gsml_data : T::GSMLHistoryDataToProcess) {
				T *instance = T::Get(gsml_data.GetTag());
				instance->LoadHistory(gsml_data);
			}
		} else {
			for (const GSMLData &gsml_data : T::GSMLHistoryDataToProcess) {
				for (const GSMLData &data_entry : gsml_data.GetChildren()) {
					//for history only data types, a new instance is created for history
					KEY identifier;
					if constexpr (std::is_same_v<KEY, int>) {
						identifier = std::stoi(data_entry.GetTag());
					} else {
						identifier = data_entry.GetTag();
					}

					T *instance = nullptr;
					if (definition) {
						instance = T::Add(identifier);
					} else {
						instance = T::Get(identifier);
						instance->LoadHistory(const_cast<GSMLData &>(data_entry));
					}
				}
			}
		}

		if (!definition) {
			T::GSMLHistoryDataToProcess.clear();
		}
	}

	/**
	**	@brief	Initialize all instances
	*/
	static void InitializeAll()
	{
		for (T *instance : T::GetAll()) {
			instance->Initialize();
		}
	}

	/**
	**	@brief	Check whether all instances are valid
	*/
	static void CheckAll()
	{
		for (const T *instance : T::GetAll()) {
			instance->Check();
		}
	}

	/**
	**	@brief	Initialize the class
	*/
	static inline bool InitializeClass()
	{
		//initialize the database parsing/processing functions for this data type
		Database::GetInstance()->AddParsingFunction(std::function<void()>(DataType::ParseDatabase));
		Database::GetInstance()->AddProcessingFunction(std::function<void(bool)>(DataType::ProcessDatabase));
		Database::GetInstance()->AddCheckingFunction(std::function<void()>(DataType::CheckAll));

		return true;
	}

private:
	static inline std::vector<T *> Instances;
	static inline std::map<KEY, std::unique_ptr<T>> InstancesByIdentifier;
	static inline int LastNumericIdentifier = 1;
	static inline std::vector<GSMLData> GSMLDataToProcess;
	static inline std::vector<GSMLData> GSMLHistoryDataToProcess;
	static inline bool ClassInitialized = DataType::InitializeClass();
};

}
