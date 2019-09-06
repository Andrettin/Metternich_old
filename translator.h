#pragma once

#include "singleton.h"

#include <QTranslator>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace metternich {

/**
**	@brief	Custom translator class
*/
class Translator : public QTranslator, public Singleton<Translator>
{
	Q_OBJECT

public:
	void LoadLocale(const std::string &language);
	std::string Translate(const std::string &source_text, const std::vector<std::string> &suffixes = std::vector<std::string>()) const;

	virtual QString translate(const char *context, const char *source_text, const char *disambiguation = nullptr, int n = -1) const override;

private:
	void AddTranslation(const std::string &source_text, const std::string &translation)
	{
		this->Translations[source_text] = translation;
	}

private:
	std::map<std::string, std::string> Translations;
};

}
