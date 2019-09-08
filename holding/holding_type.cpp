#include "holding/holding_type.h"

#include "database/gsml_data.h"
#include "script/modifier.h"

namespace metternich {

/**
**	@brief	Constructor
*/
holding_type::holding_type(const std::string &identifier) : DataEntry(identifier)
{
}

/**
**	@brief	Destructor
*/
holding_type::~holding_type()
{
}

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void holding_type::ProcessGSMLScope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "modifier") {
		this->modifier = std::make_unique<metternich::Modifier>();
		Database::ProcessGSMLData(this->modifier, scope);
	} else {
		DataEntryBase::ProcessGSMLScope(scope);
	}
}

}
