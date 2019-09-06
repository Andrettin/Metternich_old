#include "population/population_type.h"

#include "holding/holding_type.h"
#include "util.h"

namespace metternich {

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void PopulationType::ProcessGSMLScope(const gsml_data &scope)
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
		this->Color.setRgb(red, green, blue);
	} else {
		DataEntryBase::ProcessGSMLScope(scope);
	}
}

/**
**	@brief	Check whether the population type is in a valid state
*/
void PopulationType::Check() const
{
	if (!this->GetColor().isValid()) {
		throw std::runtime_error("Population type \"" + this->GetIdentifier() + "\" has no valid color.");
	}
}

QVariantList PopulationType::GetHoldingTypesQVariantList() const
{
	return ContainerToQVariantList(this->GetHoldingTypes());
}

}
