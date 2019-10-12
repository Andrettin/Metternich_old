#include "population/population_type.h"

#include "holding/holding_type.h"
#include "util/container_util.h"

namespace metternich {

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void PopulationType::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "color") {
		if (values.size() != 3) {
			throw std::runtime_error("The \"color\" scope for population types needs to contain exactly three values!");
		}

		const int red = std::stoi(values.at(0));
		const int green = std::stoi(values.at(1));
		const int blue = std::stoi(values.at(2));
		this->color.setRgb(red, green, blue);
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

/**
**	@brief	Check whether the population type is in a valid state
*/
void PopulationType::check() const
{
	if (!this->get_color().isValid()) {
		throw std::runtime_error("Population type \"" + this->get_identifier() + "\" has no valid color.");
	}
}

QVariantList PopulationType::get_holding_types_qvariant_list() const
{
	return util::container_to_qvariant_list(this->get_holding_types());
}

}
