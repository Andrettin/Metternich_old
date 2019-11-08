#include "holding/building.h"

#include "holding/holding_type.h"
#include "script/condition/and_condition.h"
#include "util/container_util.h"

namespace metternich {

/**
**	@brief	Constructor
**
**	@param	identifier	The building's string identifier
*/
building::building(const std::string &identifier) : data_entry(identifier)
{
}

/**
**	@brief	Destructor
*/
building::~building()
{
}

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void building::process_gsml_scope(const gsml_data &scope)
{
	if (scope.get_tag() == "preconditions") {
		this->preconditions = std::make_unique<and_condition>();
		database::process_gsml_data(this->preconditions.get(), scope);
	} else if (scope.get_tag() == "conditions") {
		this->conditions = std::make_unique<and_condition>();
		database::process_gsml_data(this->conditions.get(), scope);
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

/**
**	@brief	Get the building's holding types as a QVariantList
**
**	@return	The holding types as a QVariantList
*/
QVariantList building::get_holding_types_qvariant_list() const
{
	return util::container_to_qvariant_list(this->get_holding_types());
}

/**
**	@brief	Add a holding type for the building
**
**	@param	holding_type	The holding type
*/
void building::add_holding_type(holding_type *holding_type)
{
	this->holding_types.insert(holding_type);
	holding_type->add_building(this);
}

/**
**	@brief	Remove a holding type for the building
**
**	@param	holding_type	The holding type
*/
void building::remove_holding_type(holding_type *holding_type)
{
	this->holding_types.erase(holding_type);
	holding_type->remove_building(this);
}

/**
**	@brief	Get whether the building is available at all for a holding (i.e. whether it shows up on its building list)
**
**	@param	holding	The holding
**
**	@return	True if the building is available for the holding, or false otherwise
*/
bool building::is_available_for_holding(const holding *holding) const
{
	if (!this->preconditions) {
		return true;
	}

	return this->preconditions->check(holding);
}

/**
**	@brief	Get whether the building is buildable in a holding
**
**	@param	holding	The holding
**
**	@return	True if the building is buildable in the holding, or false otherwise
*/
bool building::is_buildable_in_holding(const holding *holding) const
{
	if (!this->is_available_for_holding(holding)) {
		return false;
	}

	if (!this->conditions) {
		return true;
	}

	return this->conditions->check(holding);
}

}
