#include "province.h"

#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "landed_title.h"

/**
**	@brief	Get an instance of the class by the RGB value associated with it
**
**	@param	rgb	The instance's RGB
**
**	@return	The instance if found, or null otherwise
*/
Province *Province::Get(const QRgb &rgb)
{
	typename std::map<QRgb, Province *>::const_iterator find_iterator = Province::InstancesByRgb.find(rgb);

	if (find_iterator != Province::InstancesByRgb.end()) {
		return find_iterator->second;
	}

	return nullptr;
}

/**
**	@brief	Add a new instance of the class
**
**	@param	identifier	The instance's identifier
**
**	@return	The new instance
*/
Province *Province::Add(const std::string &identifier)
{
	if (identifier.substr(0, 2) != Province::Prefix) {
		throw std::runtime_error("Invalid identifier for new province: \"" + identifier + "\". Province identifiers must begin with \"" + Province::Prefix + "\".");
	}

	return DataType<Province>::Add(identifier);
}

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

	if (key == "county") {
		if (gsml_operator == GSMLOperator::Assignment) {
			this->County = LandedTitle::Get(value);
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
		Province::InstancesByRgb[this->Color.rgb()] = this;
	} else {
		return false;
	}

	return true;
}
