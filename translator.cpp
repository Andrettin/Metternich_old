#include "translator.h"

#include "database/gsml_data.h"

#include <filesystem>

namespace Metternich {

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
