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
Commodity::Commodity(const std::string &identifier) : DataEntry(identifier)
{
}

/**
**	@brief	Destructor
*/
Commodity::~Commodity()
{
}

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void Commodity::ProcessGSMLScope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "chance") {
		this->Chance = std::make_unique<ChanceFactor>();
		Database::ProcessGSMLData(this->Chance, scope);
	} else {
		DataEntryBase::ProcessGSMLScope(scope);
	}
}

/**
**	@brief	Calculate the chance that this commodity will be produced by a given settlement holding
**
**	@param	holding	The settlement holding
*/
int Commodity::CalculateChance(holding *holding) const
{
	if (!this->Chance) {
		return 0;
	}

	//calculate the chance for the commodity to be generated as the produced one for a holding
	return this->Chance->Calculate(holding);
}

}
