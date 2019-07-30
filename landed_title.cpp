#include "landed_title.h"

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

}
