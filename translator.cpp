#include "translator.h"

#include "database/gsml_data.h"
#include "util.h"

#include <filesystem>

namespace Metternich {

QString Translator::translate(const char *context, const char *source_text, const char *disambiguation, int n) const
{
	Q_UNUSED(context)
	Q_UNUSED(n)

	if (disambiguation != nullptr) {
		std::string base_suffix_str = disambiguation;

		if (!base_suffix_str.empty()) {
			std::vector<std::string> suffixes = SplitString(base_suffix_str, ';');
			std::vector<std::string> suffix_combinations; //possible combinations of suffixes, from more specific to less specific

			for (const std::string &suffix : suffixes) {
				for (unsigned int i = 0; i < suffix_combinations.size(); i += 2) {
					suffix_combinations.insert(suffix_combinations.begin() + i, suffix_combinations[i] + "_" + suffix);
				}

				suffix_combinations.push_back("_" + suffix);
			}

			for (const std::string &suffix : suffix_combinations) {
				const auto &suffix_find_iterator = this->Translations.find(std::string(source_text) + "_" + suffix);
				if (suffix_find_iterator != this->Translations.end())  {
					return QString::fromStdString(suffix_find_iterator->second);
				}
			}
		}
	}

	const auto &find_iterator = this->Translations.find(source_text);
	if (find_iterator != this->Translations.end())  {
		return QString::fromStdString(find_iterator->second);
	}

	return source_text;
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

		GSMLData gsml_data = GSMLData::ParseFile(dir_entry.path());

		for (const GSMLProperty &property : gsml_data.GetProperties()) {
			if (property.GetOperator() != GSMLOperator::Assignment) {
				throw std::runtime_error("Only assignment operators are allowed in translation files!");
			}

			this->AddTranslation(property.GetKey(), property.GetValue());
		}
	}
}

}
