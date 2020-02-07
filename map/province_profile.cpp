#include "map/province_profile.h"

#include "map/province.h"
#include "random.h"
#include "script/condition/and_condition.h"
#include "script/context.h"

namespace metternich {

province_profile::province_profile(const std::string &identifier) : data_entry(identifier)
{
}

province_profile::~province_profile()
{
}

std::set<std::string> province_profile::get_database_dependencies()
{
	return {
		province::class_identifier,
	};
}

void province_profile::process_gsml_scope(const gsml_data &scope)
{
	if (scope.get_tag() == "conditions") {
		this->conditions = std::make_unique<and_condition<metternich::province>>();
		database::process_gsml_data(this->conditions.get(), scope);
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

void province_profile::process_gsml_dated_property(const gsml_property &property, const QDateTime &date)
{
	this->get_province()->process_gsml_dated_property(property, date);
}

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

	read_only_context ctx;

	std::vector<metternich::province *> potential_provinces;
	for (metternich::province *province : province::get_all()) {
		if (province->get_county() == nullptr) {
			continue; //only take into account provinces that belong to counties
		}

		if (!this->conditions || this->conditions->check(province, ctx)) {
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
