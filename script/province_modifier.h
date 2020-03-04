#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"
#include "script/modifier.h"

namespace metternich {

class gsml_property;
class province;

class province_modifier final : public data_entry, public data_type<province_modifier>, public modifier<province>
{
	Q_OBJECT

public:
	static constexpr const char *class_identifier = "province_modifier";
	static constexpr const char *database_folder = "province_modifiers";

public:
	province_modifier(const std::string &identifier) : data_entry(identifier) {}

	void process_gsml_property(const gsml_property &property)
	{
		modifier::process_gsml_property(property);
	}

	void process_gsml_scope(const gsml_data &scope)
	{
		modifier::process_gsml_scope(scope);
	}

	//to prevent ambiguity for name lookup between modifier::remove and data_type::remove
	void remove(province *province) const
	{
		this->modifier::remove(province);
	}
};

}
