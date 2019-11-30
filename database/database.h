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
class module;

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

	template <typename T>
	static void process_gsml_data(const std::unique_ptr<T> &instance, const gsml_data &data)
	{
		database::process_gsml_data(instance.get(), data);
	}

	static void process_gsml_property_for_object(QObject *object, const gsml_property &property);
	static QVariant process_gsml_property_value(const gsml_property &property, const QMetaProperty &meta_property, const QObject *object);

	static std::filesystem::path get_root_path()
	{
		return std::filesystem::current_path();
	}

	static std::filesystem::path get_modules_path()
	{
		return database::get_root_path() / "modules";
	}

	static std::filesystem::path get_documents_modules_path()
	{
		return database::get_documents_path() / "modules";
	}

	static std::filesystem::path get_data_path()
	{
		return database::get_root_path() / "data";
	}

	static std::filesystem::path get_common_path()
	{
		return database::get_data_path() / "common";
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
	void initialize();
	void initialize_history();
	void register_metadata(std::unique_ptr<data_type_metadata> &&metadata);
	void process_modules();
	void process_modules_at_dir(const std::filesystem::path &path, module *parent_module = nullptr);
	std::vector<std::filesystem::path> get_module_paths() const;

	module *get_module(const std::string &identifier) const
	{
		auto find_iterator = this->modules_by_identifier.find(identifier);
		if (find_iterator != this->modules_by_identifier.end()) {
			return find_iterator->second;
		}

		throw std::runtime_error("No module found with identifier \"" + identifier + "\".");
	}

	std::vector<std::filesystem::path> get_base_paths() const
	{
		std::vector<std::filesystem::path> base_paths;
		base_paths.push_back(database::get_root_path());

		std::vector<std::filesystem::path> module_paths = this->get_module_paths();
		base_paths.insert(base_paths.end(), module_paths.begin(), module_paths.end());

		return base_paths;
	}

	std::vector<std::filesystem::path> get_data_paths() const
	{
		std::vector<std::filesystem::path> paths = this->get_base_paths();

		for (std::filesystem::path &path : paths) {
			path /= "data";
		}

		return paths;
	}

	std::vector<std::filesystem::path> get_common_paths() const
	{
		std::vector<std::filesystem::path> paths = this->get_data_paths();

		for (std::filesystem::path &path : paths) {
			path /= "common";
		}

		return paths;
	}

	std::vector<std::filesystem::path> get_history_paths() const
	{
		std::vector<std::filesystem::path> paths = this->get_data_paths();

		for (std::filesystem::path &path : paths) {
			path /= "history";
		}

		return paths;
	}

	std::vector<std::filesystem::path> get_map_paths() const
	{
		std::vector<std::filesystem::path> paths = this->get_base_paths();

		for (std::filesystem::path &path : paths) {
			path /= "map";
		}

		return paths;
	}

	std::vector<std::filesystem::path> get_localization_paths() const
	{
		std::vector<std::filesystem::path> paths = this->get_base_paths();

		for (std::filesystem::path &path : paths) {
			path /= "localization";
		}

		return paths;
	}

private:
	std::vector<std::unique_ptr<data_type_metadata>> metadata;
	std::vector<std::unique_ptr<module>> modules;
	std::map<std::string, module *> modules_by_identifier;
};

}
