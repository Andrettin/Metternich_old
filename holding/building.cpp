#include "holding/building.h"

#include "holding/holding_type.h"
#include "util.h"

namespace Metternich {

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

}
