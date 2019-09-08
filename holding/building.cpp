#include "holding/building.h"

#include "holding/holding_type.h"
#include "script/condition/and_condition.h"
#include "util.h"

namespace metternich {

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
void Building::ProcessGSMLScope(const gsml_data &scope)
{
	if (scope.get_tag() == "preconditions") {
		this->Preconditions = std::make_unique<AndCondition>();
		Database::ProcessGSMLData(this->Preconditions.get(), scope);
	} else if (scope.get_tag() == "conditions") {
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
QVariantList Building::get_holding_types_qvariant_list() const
{
	return util::container_to_qvariant_list(this->get_holding_types());
}

/**
**	@brief	Add a holding type for the building
**
**	@param	holding_type	The holding type
*/
void Building::add_holding_type(holding_type *holding_type)
{
	this->holding_types.push_back(holding_type);
	holding_type->add_building(this);
}

/**
**	@brief	Remove a holding type for the building
**
**	@param	holding_type	The holding type
*/
void Building::remove_holding_type(holding_type *holding_type)
{
	this->holding_types.erase(std::remove(this->holding_types.begin(), this->holding_types.end(), holding_type), this->holding_types.end());
	holding_type->remove_building(this);
}

/**
**	@brief	Get whether the building is available at all for a holding (i.e. whether it shows up on its building list)
**
**	@param	holding	The holding
**
**	@return	True if the building is available for the holding, or false otherwise
*/
bool Building::is_available_for_holding(const holding *holding) const
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
bool Building::is_buildable_in_holding(const holding *holding) const
{
	if (!this->is_available_for_holding(holding)) {
		return false;
	}

	if (!this->Conditions) {
		return true;
	}

	return this->Conditions->Check(holding);
}

}
