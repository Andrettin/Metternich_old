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

	if (tag == "color") {
		if (values.size() != 3) {
			throw std::runtime_error("The \"color\" scope for cultures needs to contain exactly three values!");
		}

		const int red = std::stoi(values.at(0));
		const int green = std::stoi(values.at(1));
		const int blue = std::stoi(values.at(2));
		this->color.setRgb(red, green, blue);
	} else if (tag == "male_names") {
		this->male_names = util::container_to_set(values);
	} else if (tag == "female_names") {
		this->female_names = util::container_to_set(values);
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
