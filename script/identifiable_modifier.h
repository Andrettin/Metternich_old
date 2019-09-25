#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"
#include "script/modifier_base.h"

namespace metternich {

class gsml_property;

/**
**	@brief	An identifiable modifier
*/
class IdentifiableModifier : public data_entry, public data_type<IdentifiableModifier>, public ModifierBase
{
	Q_OBJECT

public:
	static constexpr const char *class_identifier = "modifier";
	static constexpr const char *database_folder = "modifiers";

	static void initialize_all()
	{
		data_type<IdentifiableModifier>::initialize_all();

		//store pointers to the hardcoded modifiers
		IdentifiableModifier::OverpopulationModifier = IdentifiableModifier::get("overpopulation");
	}

	static IdentifiableModifier *GetOverpopulationModifier()
	{
		return IdentifiableModifier::OverpopulationModifier;
	}

private:
	static inline IdentifiableModifier *OverpopulationModifier = nullptr;

public:
	IdentifiableModifier(const std::string &identifier) : data_entry(identifier) {}

	virtual void process_gsml_property(const gsml_property &property) override;

	template <typename T>
	void remove(T *scope) const //to prevent ambiguity for name lookup between ModifierBase::remove and data_type::remove
	{
		this->ModifierBase::remove(scope);
	}
};

}
