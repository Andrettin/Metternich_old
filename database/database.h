#pragma once

#include "database/gsml_data.h"
#include "singleton.h"
#include "type_traits.h"
#include "qunique_ptr.h"

#include <QStandardPaths>

#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <variant>
#include <vector>

namespace metternich {

class data_entry;
class data_type_metadata;
class module;

class database final : public singleton<database>
{
public:
	template <typename T>
	static void process_gsml_data(T *instance, const gsml_data &data)
	{
		data.for_each_element([&](const gsml_property &property) {
			instance->process_gsml_property(property);
		}, [&](const gsml_data &scope) {
			instance->process_gsml_scope(scope);
		});
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

	template <typename T>
	static void process_gsml_data(const qunique_ptr<T> &instance, const gsml_data &data)
	{
		database::process_gsml_data(instance.get(), data);
	}

	static void process_gsml_property_for_object(QObject *object, const gsml_property &property);
	static QVariant process_gsml_property_value(const gsml_property &property, const QMetaProperty &meta_property, const QObject *object);
	static void process_gsml_scope_for_object(QObject *object, const gsml_data &scope);
	static QVariant process_gsml_scope_value(const gsml_data &scope, const QMetaProperty &meta_property);

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

	static std::filesystem::path get_map_path()
	{
		return database::get_root_path() / "map";
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

	static void parse_folder(const std::filesystem::path &path, std::vector<gsml_data> &gsml_data_list);

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

	std::vector<std::filesystem::path> get_graphics_paths() const
	{
		std::vector<std::filesystem::path> paths = this->get_base_paths();

		for (std::filesystem::path &path : paths) {
			path /= "graphics";
		}

		return paths;
	}

	std::vector<std::filesystem::path> get_icons_paths() const
	{
		std::vector<std::filesystem::path> paths = this->get_graphics_paths();

		for (std::filesystem::path &path : paths) {
			path /= "icons";
		}

		return paths;
	}

	void process_icon_paths()
	{
		const std::vector<std::filesystem::path> icons_paths = this->get_icons_paths();

		for (const std::filesystem::path &path : icons_paths) {
			if (!std::filesystem::exists(path)) {
				continue;
			}

			this->process_image_paths_at_dir(path, this->icon_paths_by_tag);
		}
	}

	std::vector<std::filesystem::path> get_holding_portraits_paths() const
	{
		std::vector<std::filesystem::path> paths = this->get_graphics_paths();

		for (std::filesystem::path &path : paths) {
			path /= "holdings";
		}

		return paths;
	}

	void process_holding_portrait_paths()
	{
		const std::vector<std::filesystem::path> holding_portraits_paths = this->get_holding_portraits_paths();

		for (const std::filesystem::path &path : holding_portraits_paths) {
			if (!std::filesystem::exists(path)) {
				continue;
			}

			this->process_image_paths_at_dir(path, this->holding_portrait_paths_by_tag);
		}
	}

	std::vector<std::filesystem::path> get_flags_paths() const
	{
		std::vector<std::filesystem::path> paths = this->get_graphics_paths();

		for (std::filesystem::path &path : paths) {
			path /= "flags";
		}

		return paths;
	}

	void process_flag_paths()
	{
		const std::vector<std::filesystem::path> flags_paths = this->get_flags_paths();

		for (const std::filesystem::path &path : flags_paths) {
			if (!std::filesystem::exists(path)) {
				continue;
			}

			this->process_image_paths_at_dir(path, this->flag_paths_by_tag);
		}
	}

	std::vector<std::filesystem::path> get_texture_paths() const
	{
		std::vector<std::filesystem::path> paths = this->get_graphics_paths();

		for (std::filesystem::path &path : paths) {
			path /= "textures";
		}

		return paths;
	}

	void process_texture_paths()
	{
		const std::vector<std::filesystem::path> texture_paths = this->get_texture_paths();

		for (const std::filesystem::path &path : texture_paths) {
			if (!std::filesystem::exists(path)) {
				continue;
			}

			this->process_image_paths_at_dir(path, this->texture_paths_by_tag);
		}
	}

	void process_image_paths_at_dir(const std::filesystem::path &path, std::map<std::string, std::vector<std::filesystem::path>> &image_paths_by_tag);

	const std::filesystem::path &get_tagged_image_path(const std::map<std::string, std::vector<std::filesystem::path>> &image_paths_by_tag, const std::string &base_tag, const std::vector<std::vector<std::string>> &suffix_list_with_fallbacks = {}, const std::string &final_suffix = std::string()) const;

	const std::filesystem::path &get_tagged_icon_path(const std::string &base_tag, const std::vector<std::vector<std::string>> &suffix_list_with_fallbacks = {}, const std::string &final_suffix = std::string()) const
	{
		return this->get_tagged_image_path(this->icon_paths_by_tag, base_tag, suffix_list_with_fallbacks, final_suffix);
	}

	const std::filesystem::path &get_tagged_holding_portrait_path(const std::string &base_tag, const std::vector<std::vector<std::string>> &suffix_list_with_fallbacks = {}, const std::string &final_suffix = std::string()) const
	{
		return this->get_tagged_image_path(this->holding_portrait_paths_by_tag, base_tag, suffix_list_with_fallbacks, final_suffix);
	}

	const std::filesystem::path &get_tagged_flag_path(const std::string &base_tag, const std::vector<std::vector<std::string>> &suffix_list_with_fallbacks = {}, const std::string &final_suffix = std::string()) const
	{
		return this->get_tagged_image_path(this->flag_paths_by_tag, base_tag, suffix_list_with_fallbacks, final_suffix);
	}

	const std::filesystem::path &get_tagged_texture_path(const std::string &base_tag, const std::vector<std::vector<std::string>> &suffix_list_with_fallbacks = {}, const std::string &final_suffix = std::string()) const
	{
		return this->get_tagged_image_path(this->texture_paths_by_tag, base_tag, suffix_list_with_fallbacks, final_suffix);
	}

private:
	std::vector<std::unique_ptr<data_type_metadata>> metadata;
	std::vector<qunique_ptr<module>> modules;
	std::map<std::string, module *> modules_by_identifier;
	std::map<std::string, std::vector<std::filesystem::path>> icon_paths_by_tag;
	std::map<std::string, std::vector<std::filesystem::path>> holding_portrait_paths_by_tag;
	std::map<std::string, std::vector<std::filesystem::path>> flag_paths_by_tag;
	std::map<std::string, std::vector<std::filesystem::path>> texture_paths_by_tag;
};

}
