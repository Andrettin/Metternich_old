#pragma once

#include "singleton.h"

#include <QTranslator>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace metternich {

class translator : public QTranslator, public singleton<translator>
{
	Q_OBJECT

public:
	void load_locale(const std::string &language);
	std::string translate(const std::vector<std::string> &base_tags, const std::vector<std::vector<std::string>> &suffix_list_with_fallbacks = std::vector<std::vector<std::string>>(), const std::string &final_suffix = std::string()) const;

	std::string translate(const std::string &base_tag, const std::vector<std::vector<std::string>> &suffix_list_with_fallbacks = std::vector<std::vector<std::string>>()) const
	{
		return this->translate(std::vector<std::string>{base_tag}, suffix_list_with_fallbacks);
	}

	virtual QString translate(const char *context, const char *source_text, const char *disambiguation = nullptr, int n = -1) const override;

private:
	void add_translation(const std::string &source_text, const std::string &translation)
	{
		this->translations[source_text] = translation;
	}

private:
	std::map<std::string, std::string> translations;
};

}
