#include "translator.h"

#include "database/gsml_data.h"
#include "util.h"

#include <filesystem>

namespace metternich {

std::string Translator::Translate(const std::string &source_text, const std::vector<std::string> &suffixes) const
{
	if (!suffixes.empty()) {
		std::vector<std::string> suffix_combinations; //possible combinations of suffixes, from more specific to less specific

		for (const std::string &suffix : suffixes) {
			for (unsigned int i = 0; i < suffix_combinations.size(); i += 2) {
				suffix_combinations.insert(suffix_combinations.begin() + i, suffix_combinations[i] + "_" + suffix);
			}

			suffix_combinations.push_back("_" + suffix);
		}

		for (const std::string &suffix : suffix_combinations) {
			const auto &suffix_find_iterator = this->Translations.find(source_text + suffix);
			if (suffix_find_iterator != this->Translations.end())  {
				return suffix_find_iterator->second;
			}
		}
	}

	const auto &find_iterator = this->Translations.find(source_text);
	if (find_iterator != this->Translations.end())  {
		return find_iterator->second;
	}

	return source_text;
}

QString Translator::translate(const char *context, const char *source_text, const char *disambiguation, int n) const
{
	Q_UNUSED(context)
	Q_UNUSED(n)

	std::vector<std::string> suffixes;

	if (disambiguation != nullptr) {
		std::string base_suffix_str = disambiguation;

		if (!base_suffix_str.empty()) {
			suffixes = util::split_string(base_suffix_str, ';');
		}
	}

	return QString::fromStdString(this->Translate(source_text, suffixes));
}

void Translator::LoadLocale(const std::string &language)
{
	this->Translations.clear();

	std::filesystem::path translation_path("./localization/" + language);

	if (!std::filesystem::exists(translation_path)) {
		return;
	}

	std::filesystem::recursive_directory_iterator dir_iterator(translation_path);

	for (const std::filesystem::directory_entry &dir_entry : dir_iterator) {
		if (!dir_entry.is_regular_file()) {
			continue;
		}

		gsml_data gsml_data = gsml_data::parse_file(dir_entry.path());

		for (const gsml_property &property : gsml_data.get_properties()) {
			if (property.get_operator() != gsml_operator::assignment) {
				throw std::runtime_error("Only assignment operators are allowed in translation files!");
			}

			this->AddTranslation(property.get_key(), property.get_value());
		}
	}
}

}
