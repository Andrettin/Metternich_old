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
		this->preconditions = std::make_unique<and_condition<holding>>();
		database::process_gsml_data(this->preconditions.get(), scope);
	} else if (scope.get_tag() == "conditions") {
		this->conditions = std::make_unique<and_condition<holding>>();
		database::process_gsml_data(this->conditions.get(), scope);
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

/**
**	@brief	Get the path to the building's icon
**
**	@return	The path to the icon
*/
const std::filesystem::path &building::get_icon_path() const
{
	std::string base_tag = this->get_icon_tag();

	const std::filesystem::path &icon_path = database::get()->get_tagged_icon_path(base_tag);
	return icon_path;
}

/**
**	@brief	Get the building's holding types as a QVariantList
**
**	@return	The holding types as a QVariantList
*/
QVariantList building::get_holding_types_qvariant_list() const
{
	return container::to_qvariant_list(this->get_holding_types());
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

}
