#pragma once

#include <QTranslator>

#include <map>

/**
**	@brief	Custom translator class
*/
class Translator : public QTranslator
{
	Q_OBJECT

public:
	void LoadLanguageTranslation(const std::string &language);

	virtual QString translate(const char *context, const char *source_text, const char *disambiguation = nullptr, int n = -1) const override
	{
		Q_UNUSED(context)
		Q_UNUSED(disambiguation)
		Q_UNUSED(n)

		const auto &find_iterator = this->Translations.find(source_text);
		if (find_iterator != this->Translations.end())  {
			return QString::fromStdString(find_iterator->second);
		}

		return source_text;
	}

private:
	void AddTranslation(const std::string &source_text, const std::string &translation)
	{
		this->Translations[source_text] = translation;
	}

private:
	std::map<std::string, std::string> Translations;
};
