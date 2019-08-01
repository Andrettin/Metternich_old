#include "culture.h"

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

}
