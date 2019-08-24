#include "holding/holding_type.h"

#include "database/gsml_data.h"
#include "script/modifier.h"

namespace Metternich {

/**
**	@brief	Constructor
*/
HoldingType::HoldingType(const std::string &identifier) : DataEntry(identifier)
{
}

/**
**	@brief	Destructor
*/
HoldingType::~HoldingType()
{
}

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void HoldingType::ProcessGSMLScope(const GSMLData &scope)
{
	const std::string &tag = scope.GetTag();

	if (tag == "modifier") {
		this->Modifier = std::make_unique<Metternich::Modifier>();
		Database::ProcessGSMLData(this->Modifier, scope);
	} else {
		DataEntryBase::ProcessGSMLScope(scope);
	}
}

}
