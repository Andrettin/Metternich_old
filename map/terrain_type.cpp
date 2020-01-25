#include "map/terrain_type.h"

#include "database/gsml_data.h"
#include "engine_interface.h"
#include "script/modifier.h"
#include "util/location_util.h"
#include "util/point_util.h"

namespace metternich {

terrain_type *terrain_type::get_by_rgb(const QRgb &rgb, const bool should_find)
{
	if (rgb == terrain_type::empty_rgb) {
		return nullptr;
	}

	typename std::map<QRgb, terrain_type *>::const_iterator find_iterator = terrain_type::instances_by_rgb.find(rgb);

	if (find_iterator != terrain_type::instances_by_rgb.end()) {
		return find_iterator->second;
	}

	if (should_find) {
		throw std::runtime_error("No terrain found for RGB value: " + std::to_string(rgb) + ".");
	}

	return nullptr;
}

terrain_type::terrain_type(const std::string &identifier) : data_entry(identifier)
{
}

terrain_type::~terrain_type()
{
}

void terrain_type::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "color") {
		this->color = scope.to_color();
		terrain_type::instances_by_rgb[this->color.rgb()] = this;
	} else if (tag == "modifier") {
		this->modifier = std::make_unique<metternich::modifier<province>>();
		database::process_gsml_data(this->modifier, scope);
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

}
