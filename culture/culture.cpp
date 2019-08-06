#include "culture/culture.h"

#include "character/dynasty.h"
#include "culture/culture_group.h"
#include "random.h"

namespace Metternich {

/**
**	@brief	Check whether the culture is in a valid state
*/
void Culture::Check() const
{
	if (this->GetCultureGroup() == nullptr) {
		throw std::runtime_error("Culture \"" + this->GetIdentifier() + "\" has no culture group.");
	}

	if (this->GetMaleNames().empty() && this->GetCultureGroup()->GetMaleNames().empty()) {
		throw std::runtime_error("Culture \"" + this->GetIdentifier() + "\" has no male names, and neither does its culture group (\"" + this->GetCultureGroup()->GetIdentifier() + "\").");
	}

	if (this->GetFemaleNames().empty() && this->GetCultureGroup()->GetFemaleNames().empty()) {
		throw std::runtime_error("Culture \"" + this->GetIdentifier() + "\" has no female names, and neither does its culture group (\"" + this->GetCultureGroup()->GetIdentifier() + "\").");
	}
}

/**
**	@brief	Generate male name
*/
std::string Culture::GenerateMaleName() const
{
	if (!this->GetMaleNames().empty()) {
		return this->GetMaleNames()[Random::Generate(this->GetMaleNames().size())];
	}

	if (!this->GetCultureGroup()->GetMaleNames().empty()) {
		return this->GetCultureGroup()->GetMaleNames()[Random::Generate(this->GetCultureGroup()->GetMaleNames().size())];
	}

	return std::string();
}

/**
**	@brief	Generate female name
*/
std::string Culture::GenerateFemaleName() const
{
	if (!this->GetFemaleNames().empty()) {
		return this->GetFemaleNames()[Random::Generate(this->GetFemaleNames().size())];
	}

	if (!this->GetCultureGroup()->GetFemaleNames().empty()) {
		return this->GetCultureGroup()->GetFemaleNames()[Random::Generate(this->GetCultureGroup()->GetFemaleNames().size())];
	}

	return std::string();
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
