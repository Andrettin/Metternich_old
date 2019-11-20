#pragma once

#include "database/gsml_data.h"
#include "singleton.h"
#include "type_traits.h"

#include <QStandardPaths>

#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace metternich {

class data_entry;
class data_type_metadata;

/**
**	@brief	The database
*/
class database : public singleton<database>
{
public:
	template <typename T>
	static void process_gsml_data(T *instance, const gsml_data &data)
	{
		for (const gsml_property &property : data.get_properties()) {
			instance->process_gsml_property(property);
		}

		for (const gsml_data &child_data : data.get_children()) {
			instance->process_gsml_scope(child_data);
		}
	}

	template <typename T>
	static void process_gsml_data(T &instance, const gsml_data &data)
	{
		if constexpr (is_specialization_of_v<T, std::unique_ptr>) {
			database::process_gsml_data(instance.get(), data);
		} else {
			database::process_gsml_data(&instance, data);
		}
	}

	static void process_gsml_property_for_object(QObject *object, const gsml_property &property);

	static std::filesystem::path get_root_path()
	{
		return std::filesystem::current_path();
	}

	static std::vector<std::filesystem::path> get_base_paths()
	{
		std::vector<std::filesystem::path> base_paths;
		base_paths.push_back(database::get_root_path());

		std::vector<std::filesystem::path> module_paths = database::get_module_paths();
		base_paths.insert(base_paths.end(), module_paths.begin(), module_paths.end());

		return base_paths;
	}

	static std::filesystem::path get_modules_path()
	{
		return database::get_root_path() / "modules";
	}

	static std::filesystem::path get_documents_modules_path()
	{
		return database::get_documents_path() / "modules";
	}

	static std::vector<std::filesystem::path> get_module_paths()
	{
		std::vector<std::filesystem::path> module_paths;
		if (std::filesystem::exists(database::get_modules_path())) {
			module_paths = database::get_module_paths_at_dir(database::get_modules_path());
		}

		if (std::filesystem::exists(database::get_documents_modules_path())) {
			std::vector<std::filesystem::path> documents_module_paths = database::get_module_paths_at_dir(database::get_documents_modules_path());
			module_paths.insert(module_paths.end(), documents_module_paths.begin(), documents_module_paths.end());
		}

		return module_paths;
	}

	static std::vector<std::filesystem::path> get_module_paths_at_dir(const std::filesystem::path &path)
	{
		std::vector<std::filesystem::path> module_paths;

		std::filesystem::directory_iterator dir_iterator(path);

		for (const std::filesystem::directory_entry &dir_entry : dir_iterator) {
			if (!dir_entry.is_directory()) {
				continue;
			}

			module_paths.push_back(dir_entry);

			std::filesystem::path submodules_path = dir_entry.path() / "modules";
			if (std::filesystem::exists(submodules_path)) {
				std::vector<std::filesystem::path> submodules = get_module_paths_at_dir(submodules_path);
				module_paths.insert(module_paths.end(), submodules.begin(), submodules.end());
			}
		}

		return module_paths;
	}

	static std::filesystem::path get_data_path()
	{
		return database::get_root_path() / "data";
	}

	static std::vector<std::filesystem::path> get_data_paths()
	{
		std::vector<std::filesystem::path> paths = database::get_base_paths();

		for (std::filesystem::path &path : paths) {
			path /= "data";
		}

		return paths;
	}

	static std::filesystem::path get_common_path()
	{
		return database::get_data_path() / "common";
	}

	static std::vector<std::filesystem::path> get_common_paths()
	{
		std::vector<std::filesystem::path> paths = database::get_data_paths();

		for (std::filesystem::path &path : paths) {
			path /= "common";
		}

		return paths;
	}

	static std::vector<std::filesystem::path> get_history_paths()
	{
		std::vector<std::filesystem::path> paths = database::get_data_paths();

		for (std::filesystem::path &path : paths) {
			path /= "history";
		}

		return paths;
	}

	static std::filesystem::path get_map_path()
	{
		return database::get_root_path() / "map";
	}

	static std::filesystem::path get_graphics_path()
	{
		return database::get_root_path() / "graphics";
	}

	static std::filesystem::path get_icons_path()
	{
		return database::get_graphics_path() / "icons";
	}

	static std::filesystem::path get_building_icons_path()
	{
		return database::get_icons_path() / "buildings";
	}

	static std::filesystem::path get_commodity_icons_path()
	{
		return database::get_icons_path() / "commodities";
	}

	static std::filesystem::path get_population_icons_path()
	{
		return database::get_icons_path() / "population";
	}

	static std::filesystem::path get_holding_portraits_path()
	{
		return database::get_graphics_path() / "holdings";
	}

	static std::filesystem::path get_flags_path()
	{
		return database::get_graphics_path() / "flags";
	}

	static std::filesystem::path get_documents_path()
	{
		std::string documents_path_str = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation).toStdString();
		if (documents_path_str.empty()) {
			throw std::runtime_error("No writable documents path found.");
		}

		std::filesystem::path documents_path(documents_path_str +  "/Iron Barons");
		return documents_path;
	}

	static std::filesystem::path get_cache_path()
	{
		std::filesystem::path cache_path = database::get_documents_path();
		cache_path /= "cache";
		return cache_path;
	}

	static std::filesystem::path get_tagged_image_path(const std::filesystem::path &base_path, const std::string &base_tag, const std::vector<std::vector<std::string>> &suffix_list_with_fallbacks, const std::string &final_suffix);

public:
	database();
	~database();

	void load();
	void initialize_history();
	void register_metadata(std::unique_ptr<data_type_metadata> &&metadata);

private:
	std::vector<std::unique_ptr<data_type_metadata>> metadata;
};

}
