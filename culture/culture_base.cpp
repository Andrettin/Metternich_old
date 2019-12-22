#include "culture/culture_base.h"

#include "database/gsml_data.h"
#include "util/container_util.h"

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
		this->male_names = container::to_vector(values);
	} else if (tag == "female_names") {
		this->female_names = container::to_vector(values);
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

/**
**	@brief	Check whether the instance is in a valid state
*/
void culture_base::check() const
{
	if (!this->get_color().isValid()) {
		throw std::runtime_error("Culture \"" + this->get_identifier() + "\" has no valid color.");
	}
}

}
