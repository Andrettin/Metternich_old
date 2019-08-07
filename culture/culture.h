#pragma once

#include "culture/culture_base.h"
#include "database/data_type.h"

#include <string>
#include <vector>

namespace Metternich {

class CultureGroup;
class Dynasty;

class Culture : public CultureBase, public DataType<Culture>
{
	Q_OBJECT

	Q_PROPERTY(Metternich::CultureGroup* culture_group MEMBER CultureGroup READ GetCultureGroup NOTIFY CultureGroupChanged)

public:
	static constexpr const char *ClassIdentifier = "culture";
	static constexpr const char *DatabaseFolder = "cultures";

	Culture(const std::string &identifier) : CultureBase(identifier) {}

	virtual void Check() const override;

	CultureGroup *GetCultureGroup() const
	{
		return this->CultureGroup;
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
	std::vector<Dynasty *> Dynasties;
};

}
