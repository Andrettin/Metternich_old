#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"
#include "script/modifier.h"

namespace metternich {

class gsml_property;
class holding;

class holding_modifier final : public data_entry, public data_type<holding_modifier>, public modifier<holding>
{
	Q_OBJECT

public:
	static constexpr const char *class_identifier = "holding_modifier";
	static constexpr const char *database_folder = "holding_modifiers";

	static void initialize_all()
	{
		data_type<holding_modifier>::initialize_all();

		//store pointers to the hardcoded modifiers
		holding_modifier::overpopulation_modifier = holding_modifier::get("overpopulation");
	}

	static holding_modifier *get_overpopulation_modifier()
	{
		return holding_modifier::overpopulation_modifier;
	}

private:
	static inline holding_modifier *overpopulation_modifier = nullptr;

public:
	holding_modifier(const std::string &identifier) : data_entry(identifier) {}

	virtual void process_gsml_property(const gsml_property &property) override
	{
		modifier::process_gsml_property(property);
	}

	virtual void process_gsml_scope(const gsml_data &scope) override
	{
		modifier::process_gsml_scope(scope);
	}

	//to prevent ambiguity for name lookup between modifier::remove and data_type::remove
	void remove(holding *holding) const
	{
		this->modifier::remove(holding);
	}
};

}
