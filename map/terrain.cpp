#include "map/terrain.h"

#include "database/gsml_data.h"
#include "script/modifier.h"

namespace metternich {

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
**	@brief	Constructor
*/
Terrain::Terrain(const std::string &identifier) : data_entry(identifier)
{
}

/**
**	@brief	Destructor
*/
Terrain::~Terrain()
{
}

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void Terrain::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "color") {
		if (values.size() != 3) {
			throw std::runtime_error("The \"color\" scope for terrains needs to contain exactly three values!");
		}

		const int red = std::stoi(values.at(0));
		const int green = std::stoi(values.at(1));
		const int blue = std::stoi(values.at(2));
		this->Color.setRgb(red, green, blue);
		Terrain::InstancesByRGB[this->Color.rgb()] = this;
	} else if (tag == "modifier") {
		this->Modifier = std::make_unique<metternich::Modifier>();
		database::process_gsml_data(this->Modifier, scope);
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

}
