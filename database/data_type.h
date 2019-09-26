#pragma once

#include "database/database.h"
#include "database/data_type_base.h"
#include "database/data_type_metadata.h"
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
class data_type : public data_type_base<T>
{
public:
	static constexpr bool history_only = false; //whether the data type is defined in history only

	/**
	**	@brief	Get an instance of the class by its identifier
	**	@param	identifier	The instance's identifier
	**	@param	should_find	Whether it is expected that an instance should be found (i.e. if none is, then it is an error).
	**	@return	The instance if found, or null otherwise
	*/
	static T *get(const KEY &identifier, const bool should_find = true)
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

		auto find_iterator = data_type::instances_by_identifier.find(identifier);

		if (find_iterator != data_type::instances_by_identifier.end()) {
			return find_iterator->second.get();
		}

		if (should_find) {
			if constexpr (std::is_arithmetic_v<KEY>) {
				throw std::runtime_error("Invalid \"" + std::string(T::class_identifier) + "\" instance: \"" + std::to_string(identifier) + "\".");
			} else {
				throw std::runtime_error("Invalid \"" + std::string(T::class_identifier) + "\" instance: \"" + identifier + "\".");
			}
		}

		return nullptr;
	}

	/**
	**	@brief	Get an instance of the class by its identifier, or create it if it doesn't already exist
	**	@param	identifier	The instance's identifier
	**	@return	The instance if found, or a newly-created one otherwise
	*/
	static T *get_or_add(const KEY &identifier)
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

		typename std::map<KEY, std::unique_ptr<T>>::const_iterator find_iterator = data_type::instances_by_identifier.find(identifier);

		if (find_iterator != data_type::instances_by_identifier.end()) {
			return find_iterator->second.get();
		}

		return T::add(identifier);
	}

	/**
	**	@brief	Gets all instances of the class
	**
	**	@return	All existing instances of the class
	*/
	static const std::vector<T *> &get_all()
	{
		return data_type::instances;
	}

	/**
	**	@brief	Add a new instance of the class
	**
	**	@param	identifier	The instance's identifier
	**
	**	@return	The new instance
	*/
	static T *add(const KEY &identifier)
	{
		if constexpr (std::is_same_v<KEY, std::string>) {
			if (identifier.empty()) {
				throw std::runtime_error("Tried to add a \"" + std::string(T::class_identifier) + "\" instance with an empty string identifier.");
			}
		}

		data_type::instances_by_identifier[identifier] = std::make_unique<T>(identifier);
		T *instance = data_type::instances_by_identifier.find(identifier)->second.get();
		data_type::instances.push_back(instance);
		instance->moveToThread(QApplication::instance()->thread());

		if constexpr (std::is_same_v<KEY, int>) {
			if (identifier > data_type::last_numeric_identifier) {
				data_type::last_numeric_identifier = identifier;
			}
		}

		return instance;
	}

	/**
	**	@brief	Remove an instance of the class
	**
	**	@param	instance	The instance
	*/
	static void remove(T *instance)
	{
		data_type::instances_by_identifier.erase(instance->get_identifier());
		data_type::instances.erase(std::remove(data_type::instances.begin(), data_type::instances.end(), instance), data_type::instances.end());
	}

	/**
	**	@brief	Remove the existing class instances
	*/
	static void clear()
	{
		data_type::instances.clear();
		data_type::instances_by_identifier.clear();
	}

	/**
	**	@brief	Parse the database for the data type
	*/
	static void parse_database()
	{
		std::filesystem::path database_path("./data/common/" + std::string(T::database_folder));

		if (!std::filesystem::exists(database_path)) {
			return;
		}

		std::filesystem::recursive_directory_iterator dir_iterator(database_path);

		for (const std::filesystem::directory_entry &dir_entry : dir_iterator) {
			if (!dir_entry.is_regular_file()) {
				continue;
			}

			T::gsml_data_to_process.push_back(gsml_data::parse_file(dir_entry.path()));
		}
	}

	/**
	**	@brief	Process the database for the data type
	**
	**	@param	definition	Whether data entries are only being defined, or if their properties are actually being processed.
	*/
	static void process_database(const bool definition)
	{
		for (const gsml_data &data : T::gsml_data_to_process) {
			for (const gsml_data &data_entry : data.get_children()) {
				KEY identifier;
				if constexpr (std::is_same_v<KEY, int>) {
					identifier = std::stoi(data_entry.get_tag());
				} else {
					identifier = data_entry.get_tag();
				}

				T *instance = nullptr;
				if (definition) {
					instance = T::add(identifier);
				} else {
					instance = T::get(identifier);
					database::process_gsml_data<T>(instance, data_entry);
				}
			}
		}

		if (!definition) {
			T::gsml_data_to_process.clear();
		}
	}

	static int generate_numeric_identifier()
	{
		return ++data_type::last_numeric_identifier;
	}

	/**
	**	@brief	Parse the history database for the class
	*/
	static void parse_history_database()
	{
		std::filesystem::path history_path("./data/history/" + std::string(T::database_folder));

		if (!std::filesystem::exists(history_path)) {
			return;
		}

		//non-history only data types have files with the same name as their identifiers, while for history only data types the file name is not relevant, with the identifier being scoped to within a file
		if constexpr (T::history_only == false) {
			for (T *instance : T::get_all()) {
				std::filesystem::path history_file_path(history_path.string() + "/" + instance->get_identifier() + ".txt");

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
	static void process_history_database(const bool definition)
	{
		//non-history only data types have files with the same name as their identifiers, while for history only data types the file name is not relevant, with the identifier being scoped to within a file
		if constexpr (T::history_only == false) {
			if (definition) {
				return;
			}

			for (gsml_data &data : T::gsml_history_data_to_process) {
				T *instance = T::get(data.get_tag());
				instance->load_history(data);
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
						instance = T::add(identifier);
					} else {
						instance = T::get(identifier);
						instance->load_history(const_cast<gsml_data &>(data_entry));
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
	static void initialize_all()
	{
		for (T *instance : T::get_all()) {
			if (instance->is_initialized()) {
				continue; //the instance might have been initialized already, e.g. in the initialization function of another instance which needs it to be initialized
			}

			instance->initialize();
		}
	}

	/**
	**	@brief	Initialize all instances' history
	*/
	static void initialize_all_history()
	{
		for (T *instance : T::get_all()) {
			instance->initialize_history();
		}
	}

	/**
	**	@brief	Check whether all instances are valid
	*/
	static void check_all()
	{
		for (const T *instance : T::get_all()) {
			instance->check();
		}
	}

private:
	/**
	**	@brief	Get the database dependencies for this class
	*/
	static inline std::set<std::string> get_database_dependencies()
	{
		return {};
	}

	/**
	**	@brief	Initialize the class
	*/
	static inline bool initialize_class()
	{
		//initialize the metadata (including database parsing/processing functions) for this data type
		auto metadata = std::make_unique<data_type_metadata>(T::class_identifier, T::get_database_dependencies(), T::parse_database, T::process_database, T::check_all, T::initialize_all, T::initialize_all_history);
		database::get()->register_metadata(std::move(metadata));

		return true;
	}

private:
	static inline std::vector<T *> instances;
	static inline std::map<KEY, std::unique_ptr<T>> instances_by_identifier;
	static inline int last_numeric_identifier = 1;
	static inline std::vector<gsml_data> gsml_data_to_process;
	static inline std::set<std::string> database_dependencies; //the other classes on which this one depends, i.e. after which this class' database can be processed
#ifdef __GNUC__
	//the "used" attribute is needed under GCC, or else this variable will be optimized away (even in debug builds)
	static inline bool class_initialized [[gnu::used]] = data_type::initialize_class();
#else
	static inline bool class_initialized = data_type::initialize_class();
#endif
};

}
