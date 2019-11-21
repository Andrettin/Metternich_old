#include "map/province_profile.h"

#include "map/province.h"
#include "random.h"
#include "script/condition/and_condition.h"

namespace metternich {

/**
**	@brief	Constructor
**
**	@param	identifier	The province profile's string identifier
*/
province_profile::province_profile(const std::string &identifier) : data_entry(identifier)
{
}

/**
**	@brief	Destructor
*/
province_profile::~province_profile()
{
}

/**
**	@brief	Get the string identifiers of the classes on which this one depends for loading its database
**
**	@return	The class identifier string list
*/
std::set<std::string> province_profile::get_database_dependencies()
{
	return {
		province::class_identifier,
	};
}

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void province_profile::process_gsml_scope(const gsml_data &scope)
{
	if (scope.get_tag() == "conditions") {
		this->conditions = std::make_unique<and_condition>();
		database::process_gsml_data(this->conditions.get(), scope);
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

/**
**	@brief	Process a GSML history property
**
**	@param	property	The property
**	@param	date		The date of the property change
*/
void province_profile::process_gsml_dated_property(const gsml_property &property, const QDateTime &date)
{
	this->get_province()->process_gsml_dated_property(property, date);
}

/**
**	@brief	Process a GSML history scope
**
**	@param	scope	The scope
**	@param	date	The date of the scope change
*/
void province_profile::process_gsml_dated_scope(const gsml_data &scope, const QDateTime &date)
{
	this->get_province()->process_gsml_dated_scope(scope, date);
}

/**
**	@brief	Get the profile's province, resolving it if that hasn't been done yet
*/
province *province_profile::get_province()
{
	if (this->province != nullptr) {
		return this->province;
	}

	std::vector<metternich::province *> potential_provinces;
	for (metternich::province *province : province::get_all()) {
		if (!this->conditions || this->conditions->check(province)) {
			potential_provinces.push_back(province);
		}
	}

	if (potential_provinces.empty()) {
		throw std::runtime_error("No provinces fulfill the condition for province profile \"" + this->get_identifier() + "\".");
	}

	this->province = potential_provinces[random::generate(potential_provinces.size())];
	return this->province;
}

}
