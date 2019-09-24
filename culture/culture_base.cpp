#include "culture/culture_base.h"

#include "database/gsml_data.h"

namespace metternich {

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void culture_base::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "male_names") {
		this->male_names = values;
	} else if (tag == "female_names") {
		this->female_names = values;
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

}
