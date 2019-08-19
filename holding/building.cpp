#include "holding/building.h"

#include "holding/holding_type.h"
#include "script/condition/and_condition.h"
#include "util.h"

namespace Metternich {

/**
**	@brief	Constructor
**
**	@param	identifier	The building's string identifier
*/
Building::Building(const std::string &identifier) : DataEntry(identifier)
{
}

/**
**	@brief	Destructor
*/
Building::~Building()
{
}

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void Building::ProcessGSMLScope(const GSMLData &scope)
{
	if (scope.GetTag() == "preconditions") {
		this->Preconditions = std::make_unique<AndCondition>();
		Database::ProcessGSMLData(this->Preconditions.get(), scope);
	} else if (scope.GetTag() == "conditions") {
		this->Conditions = std::make_unique<AndCondition>();
		Database::ProcessGSMLData(this->Conditions.get(), scope);
	} else {
		DataEntryBase::ProcessGSMLScope(scope);
	}
}

/**
**	@brief	Get the building's holding types as a QVariantList
**
**	@return	The holding types as a QVariantList
*/
QVariantList Building::GetHoldingTypesQVariantList() const
{
	return ContainerToQVariantList(this->GetHoldingTypes());
}

/**
**	@brief	Add a holding type for the building
**
**	@param	holding_type	The holding type
*/
Q_INVOKABLE void Building::AddHoldingType(HoldingType *holding_type)
{
	this->HoldingTypes.push_back(holding_type);
	holding_type->AddBuilding(this);
}

/**
**	@brief	Remove a holding type for the building
**
**	@param	holding_type	The holding type
*/
Q_INVOKABLE void Building::RemoveHoldingType(HoldingType *holding_type)
{
	this->HoldingTypes.erase(std::remove(this->HoldingTypes.begin(), this->HoldingTypes.end(), holding_type), this->HoldingTypes.end());
	holding_type->RemoveBuilding(this);
}

/**
**	@brief	Get whether the building is available at all for a holding (i.e. whether it shows up on its building list)
**
**	@param	holding	The holding
**
**	@return	True if the building is available for the holding, or false otherwise
*/
bool Building::IsAvailableForHolding(const Holding *holding) const
{
	if (!this->Preconditions) {
		return true;
	}

	return this->Preconditions->Check(holding);
}

/**
**	@brief	Get whether the building is buildable in a holding
**
**	@param	holding	The holding
**
**	@return	True if the building is buildable in the holding, or false otherwise
*/
bool Building::IsBuildableInHolding(const Holding *holding) const
{
	if (!this->IsAvailableForHolding(holding)) {
		return false;
	}

	if (!this->Conditions) {
		return true;
	}

	return this->Conditions->Check(holding);
}

}
