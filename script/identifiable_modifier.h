#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"
#include "script/modifier_base.h"

namespace metternich {

class gsml_property;

/**
**	@brief	An identifiable modifier
*/
class identifiable_modifier : public data_entry, public data_type<identifiable_modifier>, public ModifierBase
{
	Q_OBJECT

public:
	static constexpr const char *class_identifier = "modifier";
	static constexpr const char *database_folder = "modifiers";

	static void initialize_all()
	{
		data_type<identifiable_modifier>::initialize_all();

		//store pointers to the hardcoded modifiers
		identifiable_modifier::overpopulation_modifier = identifiable_modifier::get("overpopulation");
	}

	static identifiable_modifier *get_overpopulation_modifier()
	{
		return identifiable_modifier::overpopulation_modifier;
	}

private:
	static inline identifiable_modifier *overpopulation_modifier = nullptr;

public:
	identifiable_modifier(const std::string &identifier) : data_entry(identifier) {}

	virtual void process_gsml_property(const gsml_property &property) override;

	template <typename T>
	void remove(T *scope) const //to prevent ambiguity for name lookup between ModifierBase::remove and data_type::remove
	{
		this->ModifierBase::remove(scope);
	}
};

}
