#include "defines.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_parser.h"
#include "game/game_speed.h"
#include "history/history.h"

namespace metternich {

defines::defines() : default_game_speed(game_speed::normal)
{
}

void defines::load(const std::filesystem::path &data_path)
{
	std::filesystem::path defines_path(data_path / "common" / "defines.txt");

	if (!std::filesystem::exists(defines_path)) {
		return;
	}

	gsml_parser parser(defines_path);
	const gsml_data gsml_data = parser.parse();

	gsml_data.for_each_property([&](const gsml_property &property) {
		this->process_gsml_property(property);
	});
}

void defines::process_gsml_property(const gsml_property &property)
{
	if (property.get_key() == "default_game_speed") {
		this->default_game_speed = string_to_game_speed(property.get_value());
	} else {
		database::process_gsml_property_for_object(this, property);
	}
}

}
