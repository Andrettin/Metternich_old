#pragma once

#include "database/data_entry.h"

#include <string>
#include <vector>

namespace Metternich {

/**
**	@brief	The base class for characteristics common to cultures and culture groups
*/
class CultureBase : public DataEntry
{
	Q_OBJECT

public:
	CultureBase(const std::string &identifier) : DataEntry(identifier) {}

	virtual void ProcessGSMLScope(const GSMLData &scope) override;

	const std::vector<std::string> &GetMaleNames() const
	{
		return this->MaleNames;
	}

	const std::vector<std::string> &GetFemaleNames() const
	{
		return this->FemaleNames;
	}

private:
	std::vector<std::string> MaleNames;
	std::vector<std::string> FemaleNames;
};

}
