#pragma once

#include "database/database.h"
#include "database/data_type_base.h"
#include "database/gsml_data.h"

#include <QApplication>

#include <filesystem>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace metternich {

/**
**	@brief	The class for the data types, which use identifiers to be referred to
*/
template <typename T, typename KEY = std::string>
class DataType : public DataTypeBase<T>
{
public:
	static constexpr bool HistoryOnly = false; //whether the data type is defined in history only

	/**
	**	@brief	Get an instance of the class by its identifier
	**	@param	identifier	The instance's identifier
	**	@param	should_find	Whether it is expected that an instance should be found (i.e. if none is, then it is an error).
	**	@return	The instance if found, or null otherwise
	*/
	static T *Get(const KEY &identifier, const bool should_find = true)
	{
		if constexpr (std::is_same_v<KEY, std::string>) {
			if (identifier == "none") {
				return nullptr;
			}
		} else if constexpr (std::is_arithmetic_v<KEY>) {
			if (identifier == 0) {
				return nullptr;
			}
		}

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
		instance->moveToThread(QApplication::instance()->thread());

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

			T::GSMLDataToProcess.push_back(gsml_data::parse_file(dir_entry.path()));
		}
	}

	/**
	**	@brief	Process the database for the data type
	**
	**	@param	definition	Whether data entries are only being defined, or if their properties are actually being processed.
	*/
	static void ProcessDatabase(const bool definition)
	{
		for (const gsml_data &data : T::GSMLDataToProcess) {
			for (const gsml_data &data_entry : data.get_children()) {
				KEY identifier;
				if constexpr (std::is_same_v<KEY, int>) {
					identifier = std::stoi(data_entry.get_tag());
				} else {
					identifier = data_entry.get_tag();
				}

				T *instance = nullptr;
				if (definition) {
					instance = T::Add(identifier);
				} else {
					instance = T::Get(identifier);
					Database::ProcessGSMLData<T>(instance, data_entry);
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

				T::gsml_history_data_to_process.push_back(gsml_data::parse_file(history_file_path));
			}
		} else {
			std::filesystem::recursive_directory_iterator dir_iterator(history_path);

			for (const std::filesystem::directory_entry &dir_entry : dir_iterator) {
				if (!dir_entry.is_regular_file()) {
					continue;
				}

				T::gsml_history_data_to_process.push_back(gsml_data::parse_file(dir_entry.path()));
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

			for (gsml_data &data : T::gsml_history_data_to_process) {
				T *instance = T::Get(data.get_tag());
				instance->LoadHistory(data);
			}
		} else {
			for (const gsml_data &data : T::gsml_history_data_to_process) {
				for (const gsml_data &data_entry : data.get_children()) {
					//for history only data types, a new instance is created for history
					KEY identifier;
					if constexpr (std::is_same_v<KEY, int>) {
						identifier = std::stoi(data_entry.get_tag());
					} else {
						identifier = data_entry.get_tag();
					}

					T *instance = nullptr;
					if (definition) {
						instance = T::Add(identifier);
					} else {
						instance = T::Get(identifier);
						instance->LoadHistory(const_cast<gsml_data &>(data_entry));
					}
				}
			}
		}

		if (!definition) {
			T::gsml_history_data_to_process.clear();
		}
	}

	/**
	**	@brief	Initialize all instances
	*/
	static void InitializeAll()
	{
		for (T *instance : T::GetAll()) {
			if (instance->IsInitialized()) {
				continue; //the instance might have been initialized already, e.g. in the initialization function of another instance which needs it to be initialized
			}

			instance->Initialize();
		}
	}

	/**
	**	@brief	Initialize all instances' history
	*/
	static void InitializeAllHistory()
	{
		for (T *instance : T::GetAll()) {
			instance->InitializeHistory();
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
		Database::Get()->AddParsingFunction(std::function<void()>(T::ParseDatabase));
		Database::Get()->AddProcessingFunction(std::function<void(bool)>(T::ProcessDatabase));
		Database::Get()->AddCheckingFunction(std::function<void()>(T::CheckAll));
		Database::Get()->AddInitializationFunction(std::function<void()>(T::InitializeAll));
		Database::Get()->AddHistoryInitializationFunction(std::function<void()>(T::InitializeAllHistory));

		return true;
	}

private:
	static inline std::vector<T *> Instances;
	static inline std::map<KEY, std::unique_ptr<T>> InstancesByIdentifier;
	static inline int LastNumericIdentifier = 1;
	static inline std::vector<gsml_data> GSMLDataToProcess;
#ifdef __GNUC__
	//the "used" attribute is needed under GCC, or else this variable will be optimized away (even in debug builds)
	static inline bool ClassInitialized [[gnu::used]] = DataType::InitializeClass();
#else
	static inline bool ClassInitialized = DataType::InitializeClass();
#endif
};

}
