#include "landed_title.h"

#include "character.h"
#include "culture.h"
#include "map/province.h"
#include "translator.h"

namespace Metternich {

/**
**	@brief	Add a new instance of the class
**
**	@param	identifier	The instance's identifier
**
**	@return	The new instance
*/
LandedTitle *LandedTitle::Add(const std::string &identifier)
{
	std::string identifier_prefix = identifier.substr(0, 2);
	if (identifier_prefix != LandedTitle::BaronyPrefix && identifier_prefix != LandedTitle::CountyPrefix && identifier_prefix != LandedTitle::DuchyPrefix && identifier_prefix != LandedTitle::KingdomPrefix && identifier_prefix != LandedTitle::EmpirePrefix) {
		throw std::runtime_error("Invalid identifier for new landed title: \"" + identifier + "\". Landed title identifiers must begin with one of the \"" + LandedTitle::BaronyPrefix + "\", \"" + LandedTitle::CountyPrefix + "\", \"" + LandedTitle::DuchyPrefix + "\", \"" + LandedTitle::KingdomPrefix + "\" or \"" + LandedTitle::EmpirePrefix + "\" prefixes, depending on the title's tier.");
	}

	return DataType<LandedTitle>::Add(identifier);
}

/**
**	@brief	Get the landed title's name
**
**	@return	The landed title's name
*/
std::string LandedTitle::GetName() const
{
	const Culture *culture = nullptr;

	if (this->GetHolder() != nullptr) {
		culture = this->GetHolder()->GetCulture();
	} else if (this->GetProvince() != nullptr) {
		culture = this->GetProvince()->GetCulture();
	}

	std::vector<std::string> suffixes;
	if (culture != nullptr) {
		suffixes.push_back(culture->GetIdentifier());
	}

	return Translator::GetInstance()->Translate(this->GetIdentifier(), suffixes);
}

}
