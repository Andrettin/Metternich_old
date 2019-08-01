#include "culture.h"

#include "dynasty.h"
#include "random.h"

namespace Metternich {

/**
**	@brief	Process GSML data scope
**
**	@param	scope	The scope
*/
void Culture::ProcessGSMLScope(const GSMLData &scope)
{
	const std::string &tag = scope.GetTag();
	const std::vector<std::string> &values = scope.GetValues();

	if (tag == "male_names") {
		this->MaleNames = values;
	} else if (tag == "female_names") {
		this->FemaleNames = values;
	} else {
		DataEntryBase::ProcessGSMLScope(scope);
	}
}

/**
**	@brief	Generate male name
*/
std::string Culture::GenerateMaleName() const
{
	if (this->MaleNames.empty()) {
		return std::string();
	}

	return this->MaleNames[Random::Generate(this->MaleNames.size())];
}

/**
**	@brief	Generate female name
*/
std::string Culture::GenerateFemaleName() const
{
	if (this->FemaleNames.empty()) {
		return std::string();
	}

	return this->FemaleNames[Random::Generate(this->FemaleNames.size())];
}

/**
**	@brief	Generate dynasty name
*/
std::string Culture::GenerateDynastyName() const
{
	if (this->Dynasties.empty()) {
		return std::string();
	}

	return this->Dynasties[Random::Generate(this->Dynasties.size())]->GetName();
}

}
