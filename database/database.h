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

	static std::filesystem::path get_graphics_path()
	{
		return database::get_root_path() / "graphics";
	}

	static std::filesystem::path get_icons_path()
	{
		return database::get_graphics_path() / "icons";
	}

	static std::filesystem::path get_population_icons_path()
	{
		return database::get_icons_path() / "population";
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

	static std::filesystem::path get_tagged_image_path(const std::filesystem::path &base_path, const std::string &base_tag, const std::vector<std::vector<std::string>> &suffix_list_with_fallbacks, const std::string &final_suffix)
	{
		std::vector<std::string> suffix_combinations; //possible combinations of tags, from more specific to less specific

		for (const std::vector<std::string> &suffix_with_fallbacks : suffix_list_with_fallbacks) {
			unsigned int added_suffixes = 0;
			for (unsigned int i = 0; i < suffix_combinations.size(); i += (1 + added_suffixes)) {
				added_suffixes = 0;
				for (const std::string &suffix_tag : suffix_with_fallbacks) {
					suffix_combinations.insert(suffix_combinations.begin() + i + added_suffixes, suffix_combinations[i] + "_" + suffix_tag);
					added_suffixes++;
				}
			}

			for (const std::string &suffix_tag : suffix_with_fallbacks) {
				suffix_combinations.push_back("_" + suffix_tag);
			}
		}

		suffix_combinations.push_back("");

		for (const std::string &suffix : suffix_combinations) {
			std::filesystem::path image_path = base_path / (base_tag + suffix + final_suffix);
			if (std::filesystem::exists(image_path)) {
				return image_path;
			}
		}

		throw std::runtime_error("No image found for base tag \"" + base_tag + "\" in path \"" + base_path.string() + "\".");
	}

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
