#pragma once

#include <QTranslator>

#include <map>
#include <string>
#include <vector>

namespace Metternich {

/**
**	@brief	Custom translator class
*/
class Translator : public QTranslator
{
	Q_OBJECT

public:
	void LoadLocale(const std::string &language);

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
