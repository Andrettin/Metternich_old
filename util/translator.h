#pragma once

#include "util/singleton.h"

#include <QTranslator>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace metternich {

class translator final : public QTranslator, public singleton<translator>
{
	Q_OBJECT

public:
	const std::string &get_locale() const
	{
		return this->locale;
	}

	void set_locale(const std::string &locale)
	{
		this->locale = locale;
	}

	void load();
	void load_file(const std::filesystem::path &filepath);
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
	std::string locale;
};

}
