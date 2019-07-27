#include "character.h"

#include "database/gsml_property.h"
#include "util.h"

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
**
**	@return	True if the property key is valid (and the operator is valid for it), or false otherwise
*/
bool Character::ProcessGSMLProperty(const GSMLProperty &property)
{
	const std::string &key = property.GetKey();
	const GSMLOperator gsml_operator = property.GetOperator();
	const std::string &value = property.GetValue();

	if (key == "name") {
		if (gsml_operator == GSMLOperator::Assignment) {
			this->Name = value;
		}
	} else if (key == "female") {
		if (gsml_operator == GSMLOperator::Assignment) {
			this->Female = StringToBool(value);
		}
	} else {
		return false;
	}

	return true;
}
