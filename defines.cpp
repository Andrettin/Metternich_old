#include "defines.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_parser.h"
#include "history/history.h"

namespace metternich {

/**
**	@brief	Load the defines
*/
void Defines::load()
{
	std::filesystem::path defines_path("./data/common/defines.txt");

	if (!std::filesystem::exists(defines_path)) {
		return;
	}

	gsml_parser parser(defines_path);
	gsml_data gsml_data = parser.parse();

	for (const gsml_property &property : gsml_data.get_properties()) {
		this->process_gsml_property(property);
	}
}

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void Defines::process_gsml_property(const gsml_property &property)
{
	database::process_gsml_property_for_object(this, property);
}

}
