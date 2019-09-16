#include "economy/commodity.h"

#include "database/database.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "script/chance_factor.h"

namespace metternich {

/**
**	@brief	Constructor
**
**	@param	identifier	The commodity's string identifier
*/
commodity::commodity(const std::string &identifier) : DataEntry(identifier)
{
}

/**
**	@brief	Destructor
*/
commodity::~commodity()
{
}

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void commodity::ProcessGSMLScope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "chance") {
		this->chance = std::make_unique<ChanceFactor>();
		Database::ProcessGSMLData(this->chance, scope);
	} else {
		DataEntryBase::ProcessGSMLScope(scope);
	}
}

/**
**	@brief	Calculate the chance that this commodity will be produced by a given settlement holding
**
**	@param	holding	The settlement holding
*/
int commodity::calculate_chance(holding *holding) const
{
	if (!this->chance) {
		return 0;
	}

	//calculate the chance for the commodity to be generated as the produced one for a holding
	return this->chance->Calculate(holding);
}

}
