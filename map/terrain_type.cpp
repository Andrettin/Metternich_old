#include "map/terrain_type.h"

#include "database/gsml_data.h"
#include "script/modifier.h"

namespace metternich {

/**
**	@brief	Get an instance of the class by the RGB value associated with it
**	@param	rgb	The instance's RGB
**	@param	should_find	Whether it is expected that an instance should be found (i.e. if none is, then it is an error).
**	@return	The instance if found, or null otherwise
*/
terrain_type *terrain_type::get_by_rgb(const QRgb &rgb, const bool should_find)
{
	typename std::map<QRgb, terrain_type *>::const_iterator find_iterator = terrain_type::instances_by_rgb.find(rgb);

	if (find_iterator != terrain_type::instances_by_rgb.end()) {
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
terrain_type::terrain_type(const std::string &identifier) : data_entry(identifier)
{
}

/**
**	@brief	Destructor
*/
terrain_type::~terrain_type()
{
}

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void terrain_type::process_gsml_scope(const gsml_data &scope)
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
		this->color.setRgb(red, green, blue);
		terrain_type::instances_by_rgb[this->color.rgb()] = this;
	} else if (tag == "modifier") {
		this->modifier = std::make_unique<metternich::Modifier>();
		database::process_gsml_data(this->modifier, scope);
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

}
