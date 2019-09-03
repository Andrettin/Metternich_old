#pragma once

#include "culture/culture_base.h"
#include "database/data_type.h"

#include <QColor>

#include <string>
#include <vector>

namespace Metternich {

class CultureGroup;
class Dynasty;

class Culture : public CultureBase, public DataType<Culture>
{
	Q_OBJECT

	Q_PROPERTY(Metternich::CultureGroup* culture_group MEMBER CultureGroup READ GetCultureGroup NOTIFY CultureGroupChanged)
	Q_PROPERTY(QColor color MEMBER Color READ GetColor)

public:
	static constexpr const char *ClassIdentifier = "culture";
	static constexpr const char *DatabaseFolder = "cultures";

	Culture(const std::string &identifier) : CultureBase(identifier) {}

	virtual void ProcessGSMLScope(const GSMLData &scope) override;
	virtual void Check() const override;

	Metternich::CultureGroup *GetCultureGroup() const
	{
		return this->CultureGroup;
	}

	const QColor &GetColor() const
	{
		return this->Color;
	}

	void AddDynasty(Dynasty *dynasty)
	{
		this->Dynasties.push_back(dynasty);
	}

	std::string GenerateMaleName() const;
	std::string GenerateFemaleName() const;
	std::string GenerateDynastyName() const;

signals:
	void CultureGroupChanged();

private:
	Metternich::CultureGroup *CultureGroup = nullptr;
	QColor Color;
	std::vector<Dynasty *> Dynasties;
};

}
