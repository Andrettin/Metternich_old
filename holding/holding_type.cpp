#include "holding/holding_type.h"

#include "database/gsml_data.h"
#include "script/modifier.h"

namespace metternich {

/**
**	@brief	Constructor
*/
holding_type::holding_type(const std::string &identifier) : data_entry(identifier)
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
void holding_type::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "modifier") {
		this->modifier = std::make_unique<metternich::Modifier>();
		database::process_gsml_data(this->modifier, scope);
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

}
