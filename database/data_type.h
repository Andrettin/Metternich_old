#pragma once

#include "database/database.h"
#include "database/data_type_base.h"
#include "database/data_type_metadata.h"
#include "database/gsml_data.h"
#include "database/gsml_operator.h"
#include "database/gsml_parser.h"

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
	static constexpr const char *database_base_folder = "common";

	/**
	**	@brief	Get an instance of the class by its identifier
	**	@param	identifier	The instance's identifier
	**	@return	The instance
	*/
	static T *get(const KEY &identifier)
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

		T *instance = data_type::try_get(identifier);

		if (instance == nullptr) {
			if constexpr (std::is_arithmetic_v<KEY>) {
				throw std::runtime_error("Invalid \"" + std::string(T::class_identifier) + "\" instance: \"" + std::to_string(identifier) + "\".");
			} else {
				throw std::runtime_error("Invalid \"" + std::string(T::class_identifier) + "\" instance: \"" + identifier + "\".");
			}
		}

		return instance;
	}

	/**
	**	@brief	Try to get an instance of the class by its identifier
	**	@param	identifier	The instance's identifier
	**	@return	The instance if found, or null otherwise
	*/
	static T *try_get(const KEY &identifier)
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

		auto alias_find_iterator = data_type::instances_by_alias.find(identifier);
		if (alias_find_iterator != data_type::instances_by_alias.end()) {
			return alias_find_iterator->second;
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
		T *instance = data_type::try_get(identifier);
		if (instance != nullptr) {
			return instance;
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

		if (data_type::instances_by_identifier.contains(identifier) || data_type::instances_by_alias.contains(identifier)) {
			if constexpr (std::is_arithmetic_v<KEY>) {
				throw std::runtime_error("Tried to add a \"" + std::string(T::class_identifier) + "\" instance with the already-used \"" + std::to_string(identifier) + "\" string identifier.");
			} else {
				throw std::runtime_error("Tried to add a \"" + std::string(T::class_identifier) + "\" instance with the already-used \"" + identifier + "\" string identifier.");
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

	static void add_instance_alias(T *instance, const KEY &alias)
	{
		if constexpr (std::is_same_v<KEY, std::string>) {
			if (alias.empty()) {
				throw std::runtime_error("Tried to add a \"" + std::string(T::class_identifier) + "\" instance empty alias.");
			}
		}

		if (data_type::instances_by_identifier.contains(alias) || data_type::instances_by_alias.contains(alias)) {
			if constexpr (std::is_arithmetic_v<KEY>) {
				throw std::runtime_error("Tried to add a \"" + std::string(T::class_identifier) + "\" alias with the already-used \"" + std::to_string(alias) + "\" string identifier.");
			} else {
				throw std::runtime_error("Tried to add a \"" + std::string(T::class_identifier) + "\" alias with the already-used \"" + alias + "\" string identifier.");
			}
		}

		data_type::instances_by_alias[alias] = instance;
		instance->add_alias(alias);
	}

	/**
	**	@brief	Remove an instance of the class
	**
	**	@param	instance	The instance
	*/
	static void remove(T *instance)
	{
		for (const KEY &alias : instance->get_aliases()) {
			data_type::instances_by_alias.erase(alias);
		}

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
		data_type::instances_by_alias.clear();
	}

	/**
	**	@brief	Parse the database for the data type
	*/
	static void parse_database(const std::filesystem::path &data_path)
	{
		if (std::string(T::database_folder).empty()) {
			return;
		}

		std::filesystem::path database_path(data_path / T::database_base_folder / T::database_folder);

		if (!std::filesystem::exists(database_path)) {
			return;
		}

		std::filesystem::recursive_directory_iterator dir_iterator(database_path);

		for (const std::filesystem::directory_entry &dir_entry : dir_iterator) {
			if (!dir_entry.is_regular_file()) {
				continue;
			}

			gsml_parser parser(dir_entry.path());
			T::gsml_data_to_process.push_back(parser.parse());
		}
	}

	/**
	**	@brief	Process the database for the data type
	**
	**	@param	definition	Whether data entries are only being defined, or if their properties are actually being processed.
	*/
	static void process_database(const bool definition)
	{
		if (std::string(T::database_folder).empty()) {
			return;
		}

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
					if (data_entry.get_operator() != gsml_operator::addition) { //addition operators for data entry scopes mean modifying already-defined entries
						instance = T::add(identifier);
					} else {
						instance = T::get(identifier);
					}

					for (const gsml_property *alias_property : data_entry.try_get_properties("aliases")) {
						if (alias_property->get_operator() != gsml_operator::addition) {
							throw std::runtime_error("Only the addition operator is supported for data entry aliases.");
						}

						const std::string &alias = alias_property->get_value();
						if constexpr (std::is_same_v<KEY, int>) {
							T::add_instance_alias(instance, std::stoi(alias));
						} else {
							T::add_instance_alias(instance, alias);
						}
					}
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
		if (std::string(T::database_folder).empty()) {
			return;
		}

		for (const std::filesystem::path &path : database::get()->get_history_paths()) {
			std::filesystem::path history_path(path / T::database_folder);

			if (!std::filesystem::exists(history_path)) {
				continue;
			}

			std::filesystem::recursive_directory_iterator dir_iterator(history_path);

			for (const std::filesystem::directory_entry &dir_entry : dir_iterator) {
				if (!dir_entry.is_regular_file()) {
					continue;
				}

				if constexpr (T::history_only == false) {
					//non-history only data types have files with the same name as their identifiers, while for history only data types the file name is not relevant, with the identifier being scoped to within a file
					if (T::try_get(dir_entry.path().stem().string()) == nullptr) {
						throw std::runtime_error(dir_entry.path().stem().string() + " is not a valid \"" + T::class_identifier + "\" instance identifier.");
					}
				}

				gsml_parser parser(dir_entry.path());
				T::gsml_history_data_to_process.push_back(parser.parse());
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

			for (const gsml_data &data : T::gsml_history_data_to_process) {
				T *instance = T::get(data.get_tag());
				instance->process_history(data);
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
						if (data_entry.get_operator() == gsml_operator::addition) {
							continue; //addition operators for data entry scopes mean modifying already-defined entries
						}

						instance = T::add(identifier);
					} else {
						instance = T::get(identifier);
						instance->process_history(data_entry);
					}
				}
			}
		}

		if (!definition) {
			for (T *instance : T::get_all()) {
				instance->load_history();
			}

			T::gsml_history_data_to_process.clear();
		}
	}

	/**
	**	@brief	Process the cache for the class
	*/
	static void process_cache()
	{
		if (std::string(T::database_folder).empty()) {
			return;
		}

		const std::filesystem::path cache_path(database::get_cache_path() / std::string(T::database_folder));

		if (!std::filesystem::exists(cache_path)) {
			return;
		}

		std::vector<gsml_data> cache_data_to_process;

		std::filesystem::recursive_directory_iterator dir_iterator(cache_path);

		for (const std::filesystem::directory_entry &dir_entry : dir_iterator) {
			if (!dir_entry.is_regular_file() || dir_entry.path().extension() != ".txt") {
				continue;
			}

			if (T::try_get(dir_entry.path().stem().string()) == nullptr) {
				throw std::runtime_error(dir_entry.path().stem().string() + " is not a valid \"" + T::class_identifier + "\" instance identifier.");
			}

			gsml_parser parser(dir_entry.path());
			cache_data_to_process.push_back(parser.parse());
		}

		for (gsml_data &data : cache_data_to_process) {
			T *instance = T::get(data.get_tag());
			database::process_gsml_data<T>(instance, data);
		}
	}

	/**
	**	@brief	Save the cache for the instances of the class
	*/
	static void save_cache()
	{
		if (std::string(T::database_folder).empty()) {
			return;
		}

		const std::filesystem::path cache_path(database::get_cache_path() / std::string(T::database_folder));

		if (!std::filesystem::exists(cache_path)) {
			std::filesystem::create_directories(cache_path);
		}

		std::vector<gsml_data> cache_data_list;
		for (T *instance : T::get_all()) {
			cache_data_list.push_back(instance->get_cache_data());
		}

		for (const gsml_data &cache_data : cache_data_list) {
			cache_data.print_to_dir(cache_path);
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
			if (!instance->is_history_initialized()) {
				instance->initialize_history();
			}
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
	static inline std::map<KEY, T *> instances_by_alias;
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
