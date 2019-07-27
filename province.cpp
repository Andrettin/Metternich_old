#include "province.h"

#include "database/gsml_data.h"
#include "database/gsml_property.h"

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
**
**	@return	True if the property key is valid (and the operator is valid for it), or false otherwise
*/
bool Province::ProcessGSMLProperty(const GSMLProperty &property)
{
	const std::string &key = property.GetKey();
	const GSMLOperator gsml_operator = property.GetOperator();
	const std::string &value = property.GetValue();

	if (key == "name") {
		if (gsml_operator == GSMLOperator::Assignment) {
			this->Name = value;
		}
	} else {
		return false;
	}

	return true;
}

/**
**	@brief	Process GSML data scope
**
**	@param	scope	The scope
**
**	@return	True if the scope tag is valid, or false otherwise
*/
bool Province::ProcessGSMLScope(const GSMLData &scope)
{
	const std::string &tag = scope.GetTag();
	const std::vector<std::string> &values = scope.GetValues();

	if (tag == "color") {
		if (values.size() != 3) {
			throw std::runtime_error("The \"color\" scope for provinces needs to contain exactly three values!");
		}

		const int red = std::stoi(values.at(0));
		const int green = std::stoi(values.at(1));
		const int blue = std::stoi(values.at(2));
		this->Color.setRgb(red, green, blue);
	} else {
		return false;
	}

	return true;
}
