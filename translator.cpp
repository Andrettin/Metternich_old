#include "translator.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_parser.h"
#include "util/string_util.h"

#include <filesystem>

namespace metternich {

std::string translator::translate(const std::string &base_tag, const std::vector<std::vector<std::string>> &suffix_list_with_fallbacks) const
{
	std::vector<std::string> suffix_combinations = string::get_suffix_combinations(suffix_list_with_fallbacks);

	for (const std::string &suffix : suffix_combinations) {
		const auto &suffix_find_iterator = this->translations.find(base_tag + suffix);
		if (suffix_find_iterator != this->translations.end())  {
			return suffix_find_iterator->second;
		}
	}

	return base_tag;
}

QString translator::translate(const char *context, const char *source_text, const char *disambiguation, int n) const
{
	Q_UNUSED(context)
	Q_UNUSED(n)
	Q_UNUSED(disambiguation)

	return QString::fromStdString(this->translate(source_text));
}

void translator::load_locale(const std::string &language)
{
	this->translations.clear();

	for (const std::filesystem::path &path : database::get()->get_localization_paths()) {
		std::filesystem::path localization_path(path / language);

		if (!std::filesystem::exists(localization_path)) {
			continue;
		}

		std::filesystem::recursive_directory_iterator dir_iterator(localization_path);

		for (const std::filesystem::directory_entry &dir_entry : dir_iterator) {
			if (!dir_entry.is_regular_file()) {
				continue;
			}

			gsml_parser parser(dir_entry.path());
			gsml_data gsml_data = parser.parse();

			for (const gsml_property &property : gsml_data.get_properties()) {
				if (property.get_operator() != gsml_operator::assignment) {
					throw std::runtime_error("Only assignment operators are allowed in translation files!");
				}

				this->add_translation(property.get_key(), property.get_value());
			}
		}
	}
}

}
