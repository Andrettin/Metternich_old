#include "map/terrain.h"

#include "database/gsml_data.h"

namespace Metternich {

/**
**	@brief	Get an instance of the class by the RGB value associated with it
**	@param	rgb	The instance's RGB
**	@param	should_find	Whether it is expected that an instance should be found (i.e. if none is, then it is an error).
**	@return	The instance if found, or null otherwise
*/
Terrain *Terrain::GetByRGB(const QRgb &rgb, const bool should_find)
{
	typename std::map<QRgb, Terrain *>::const_iterator find_iterator = Terrain::InstancesByRGB.find(rgb);

	if (find_iterator != Terrain::InstancesByRGB.end()) {
		return find_iterator->second;
	}

	if (should_find) {
		throw std::runtime_error("No terrain found for RGB value: " + std::to_string(rgb) + ".");
	}

	return nullptr;
}

/**
**	@brief	Process GSML data scope
**
**	@param	scope	The scope
*/
void Terrain::ProcessGSMLScope(const GSMLData &scope)
{
	const std::string &tag = scope.GetTag();
	const std::vector<std::string> &values = scope.GetValues();

	if (tag == "color") {
		if (values.size() != 3) {
			throw std::runtime_error("The \"color\" scope for terrains needs to contain exactly three values!");
		}

		const int red = std::stoi(values.at(0));
		const int green = std::stoi(values.at(1));
		const int blue = std::stoi(values.at(2));
		this->Color.setRgb(red, green, blue);
		Terrain::InstancesByRGB[this->Color.rgb()] = this;
	} else {
		DataEntryBase::ProcessGSMLScope(scope);
	}
}

}
