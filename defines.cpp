#include "defines.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "history/history.h"

namespace metternich {

/**
**	@brief	Load the defines
*/
void Defines::Load()
{
	std::filesystem::path defines_path("./data/common/defines.txt");

	if (!std::filesystem::exists(defines_path)) {
		return;
	}

	gsml_data gsml_data = gsml_data::parse_file(defines_path);

	for (const gsml_property &property : gsml_data.get_properties()) {
		this->ProcessGSMLProperty(property);
	}
}

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void Defines::ProcessGSMLProperty(const gsml_property &property)
{
	Database::ProcessGSMLPropertyForObject(this, property);
}

}
