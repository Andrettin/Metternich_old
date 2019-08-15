#include "defines.h"

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
		Defines::ProcessGSMLProperty(property);
	}
}

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void Defines::ProcessGSMLProperty(const GSMLProperty &property)
{
	const std::string &key = property.GetKey();
	const GSMLOperator gsml_operator = property.GetOperator();
	const std::string &value = property.GetValue();

	if (key == "start_date") {
		if (gsml_operator == GSMLOperator::Assignment) {
			Defines::StartDate = History::StringToDate(value);
		} else {
			throw std::runtime_error("Invalid operator for define (\"" + property.GetKey() + "\").");
		}
	} else if (key == "player_character") {
		if (gsml_operator == GSMLOperator::Assignment) {
			Defines::PlayerCharacterID = std::stoi(value);
		} else {
			throw std::runtime_error("Invalid operator for define (\"" + property.GetKey() + "\").");
		}
	} else {
		throw std::runtime_error("Invalid define: \"" + property.GetKey() + "\".");
	}
}

}
