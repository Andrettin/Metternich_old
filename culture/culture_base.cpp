#include "culture/culture_base.h"

#include "database/gsml_data.h"

namespace Metternich {

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void CultureBase::ProcessGSMLScope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "male_names") {
		this->MaleNames = values;
	} else if (tag == "female_names") {
		this->FemaleNames = values;
	} else {
		DataEntryBase::ProcessGSMLScope(scope);
	}
}

}
