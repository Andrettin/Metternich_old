#include "religion.h"

namespace Metternich {

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void Religion::ProcessGSMLScope(const GSMLData &scope)
{
	const std::string &tag = scope.GetTag();
	const std::vector<std::string> &values = scope.GetValues();

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
**	@brief	Check whether the religion is in a valid state
*/
void Religion::Check() const
{
	if (!this->GetColor().isValid()) {
		throw std::runtime_error("Religion \"" + this->GetIdentifier() + "\" has no valid color.");
	}
}

}
