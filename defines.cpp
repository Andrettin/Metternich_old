#include "defines.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "history/history.h"

namespace Metternich {

/**
**	@brief	Load the defines
*/
void Defines::Load()
{
	std::filesystem::path defines_path("./data/common/defines.txt");

	if (!std::filesystem::exists(defines_path)) {
		return;
	}

	GSMLData gsml_data = GSMLData::ParseFile(defines_path);

	for (const GSMLProperty &property : gsml_data.GetProperties()) {
		this->ProcessGSMLProperty(property);
	}
}

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void Defines::ProcessGSMLProperty(const GSMLProperty &property)
{
	Database::ProcessGSMLPropertyForObject(this, property);
}

}
