#include "economy/commodity.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"

namespace Metternich {

/**
**	@brief	Process GSML data scope
**
**	@param	scope	The scope
*/
void Commodity::ProcessGSMLScope(const GSMLData &scope)
{
	const std::string &tag = scope.GetTag();

	if (tag == "chance") {
		Database::ProcessGSMLData(this->Chance, scope);
	} else {
		DataEntryBase::ProcessGSMLScope(scope);
	}
}

}
