#pragma once

#include "database/database.h"
#include "database/data_type_base.h"
#include "database/data_type_metadata.h"
#include "database/gsml_data.h"
#include "database/gsml_operator.h"
#include "database/gsml_parser.h"

#include <QApplication>
#include <QUuid>

#include <filesystem>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace metternich {

template <typename T>
class data_type : public data_type_base<T>
{
public:
	static constexpr bool history_only = false; //whether the data type is defined in history only
	static constexpr const char *database_base_folder = "common";

	static T *get(const std::string &identifier)
	{
		if (identifier == "none") {
			return nullptr;
		}

		T *instance = data_type::try_get(identifier);

		if (instance == nullptr) {
			throw std::runtime_error("Invalid " + std::string(T::class_identifier) + " instance: \"" + identifier + "\".");
		}

		return instance;
	}

	static T *try_get(const std::string &identifier)
	{
		if (identifier == "none") {
			return nullptr;
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

	static T *get_or_add(const std::string &identifier)
	{
		T *instance = data_type::try_get(identifier);
		if (instance != nullptr) {
			return instance;
		}

		return T::add(identifier);
	}

	static const std::vector<T *> &get_all()
	{
		return data_type::instances;
	}

	static T *add(const std::string &identifier)
	{
		if (identifier.empty()) {
			throw std::runtime_error("Tried to add a \"" + std::string(T::class_identifier) + "\" instance with an empty string identifier.");
		}

		if (data_type::instances_by_identifier.contains(identifier) || data_type::instances_by_alias.contains(identifier)) {
			throw std::runtime_error("Tried to add a \"" + std::string(T::class_identifier) + "\" instance with the already-used \"" + identifier + "\" string identifier.");
		}

		data_type::instances_by_identifier[identifier] = std::make_unique<T>(identifier);
		T *instance = data_type::instances_by_identifier.find(identifier)->second.get();
		data_type::instances.push_back(instance);
		instance->moveToThread(QApplication::instance()->thread());

		return instance;
	}

	static void add_instance_alias(T *instance, const std::string &alias)
	{
		if (alias.empty()) {
			throw std::runtime_error("Tried to add a \"" + std::string(T::class_identifier) + "\" instance empty alias.");
		}

		if (data_type::instances_by_identifier.contains(alias) || data_type::instances_by_alias.contains(alias)) {
			throw std::runtime_error("Tried to add a \"" + std::string(T::class_identifier) + "\" alias with the already-used \"" + alias + "\" string identifier.");
		}

		data_type::instances_by_alias[alias] = instance;
		instance->add_alias(alias);
	}

	static void remove(T *instance)
	{
		for (const std::string &alias : instance->get_aliases()) {
			data_type::instances_by_alias.erase(alias);
		}

		data_type::instances_by_identifier.erase(instance->get_identifier());

		data_type::instances.erase(std::remove(data_type::instances.begin(), data_type::instances.end(), instance), data_type::instances.end());
	}

	static void clear()
	{
		data_type::instances.clear();
		data_type::instances_by_identifier.clear();
		data_type::instances_by_alias.clear();
	}

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

	static void process_database(const bool definition)
	{
		if (std::string(T::database_folder).empty()) {
			return;
		}

		for (const gsml_data &data : T::gsml_data_to_process) {
			for (const gsml_data &data_entry : data.get_children()) {
				const std::string &identifier = data_entry.get_tag();

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
						T::add_instance_alias(instance, alias);
					}
				} else {
					try {
						instance = T::get(identifier);
						database::process_gsml_data<T>(instance, data_entry);
					} catch (...) {
						std::throw_with_nested(std::runtime_error("Error processing or loading data for \"" + std::string(T::class_identifier) + "\" instance \"" + identifier + "\"."));
					}
				}
			}
		}

		if (!definition) {
			T::gsml_data_to_process.clear();
		}
	}

	static std::string generate_identifier()
	{
		std::string identifier;

		while (identifier.empty() || data_type::try_get(identifier) != nullptr) {
			QUuid uuid = QUuid::createUuid();
			identifier = uuid.toString(QUuid::WithoutBraces).toStdString();
		}

		return identifier;
	}

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
						throw std::runtime_error("Could not parse history for \"" + dir_entry.path().stem().string() + "\", as it is not a valid " + T::class_identifier + " instance identifier.");
					}
				}

				gsml_parser parser(dir_entry.path());
				T::gsml_history_data_to_process.push_back(parser.parse());
			}
		}
	}

	static void process_history_database(const bool definition)
	{
		//non-history only data types have files with the same name as their identifiers, while for history only data types the file name is not relevant, with the identifier being scoped to within a file
		if constexpr (T::history_only == false) {
			if (definition) {
				return;
			}

			for (const gsml_data &data : T::gsml_history_data_to_process) {
				try {
					T *instance = T::get(data.get_tag());
					instance->process_history(data);
				} catch (...) {
					std::throw_with_nested(std::runtime_error("Error processing history data for \"" + std::string(T::class_identifier) + "\" instance \"" + data.get_tag() + "\"."));
				}
			}
		} else {
			for (const gsml_data &data : T::gsml_history_data_to_process) {
				for (const gsml_data &data_entry : data.get_children()) {
					//for history only data types, a new instance is created for history
					const std::string &identifier = data_entry.get_tag();

					T *instance = nullptr;

					try {
						if (definition) {
							if (data_entry.get_operator() == gsml_operator::addition) {
								continue; //addition operators for data entry scopes mean modifying already-defined entries
							}

							instance = T::add(identifier);
						} else {
							instance = T::get(identifier);
							instance->process_history(data_entry);
						}
					} catch (...) {
						std::throw_with_nested(std::runtime_error("Error processing history data for \"" + std::string(T::class_identifier) + "\" instance \"" + identifier + "\"."));
					}
				}
			}
		}

		if (!definition) {
			for (T *instance : T::get_all()) {
				try {
					instance->load_history();
				} catch (...) {
					std::throw_with_nested(std::runtime_error("Error loading history data for " + std::string(T::class_identifier) + " instance \"" + instance->get_identifier() + "\"."));
				}
			}

			T::gsml_history_data_to_process.clear();
		}
	}

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
				throw std::runtime_error("Could not process the cache for \"" + dir_entry.path().stem().string() + "\", as it is not a valid " + T::class_identifier + " instance identifier.");
			}

			gsml_parser parser(dir_entry.path());
			cache_data_to_process.push_back(parser.parse());
		}

		for (gsml_data &data : cache_data_to_process) {
			T *instance = T::get(data.get_tag());
			database::process_gsml_data<T>(instance, data);
		}
	}

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

	static void initialize_all()
	{
		for (T *instance : T::get_all()) {
			if (instance->is_initialized()) {
				continue; //the instance might have been initialized already, e.g. in the initialization function of another instance which needs it to be initialized
			}

			instance->initialize();
		}
	}

	static void initialize_all_history()
	{
		for (T *instance : T::get_all()) {
			if (!instance->is_history_initialized()) {
				instance->initialize_history();
			}
		}
	}

	static void check_all()
	{
		for (const T *instance : T::get_all()) {
			try {
				instance->check();
			} catch (...) {
				std::throw_with_nested(std::runtime_error("The " + std::string(std::string(T::class_identifier)) + " instance \"" + instance->get_identifier() + "\" is in an invalid state."));
			}
		}
	}

	static void check_all_history()
	{
		for (const T *instance : T::get_all()) {
			try {
				instance->check_history();
			} catch (...) {
				std::throw_with_nested(std::runtime_error("The " + std::string(std::string(T::class_identifier)) + " instance \"" + instance->get_identifier() + "\" is in an invalid state."));
			}
		}
	}

private:
	static inline std::set<std::string> get_database_dependencies()
	{
		return {};
	}

	static inline bool initialize_class()
	{
		//initialize the metadata (including database parsing/processing functions) for this data type
		auto metadata = std::make_unique<data_type_metadata>(T::class_identifier, T::get_database_dependencies(), T::parse_database, T::process_database, T::check_all, T::check_all_history, T::initialize_all, T::initialize_all_history);
		database::get()->register_metadata(std::move(metadata));

		return true;
	}

private:
	static inline std::vector<T *> instances;
	static inline std::map<std::string, std::unique_ptr<T>> instances_by_identifier;
	static inline std::map<std::string, T *> instances_by_alias;
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
