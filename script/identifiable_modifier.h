#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"
#include "script/modifier_base.h"

namespace metternich {

class gsml_property;

/**
**	@brief	An identifiable modifier
*/
class IdentifiableModifier : public data_entry, public DataType<IdentifiableModifier>, public ModifierBase
{
	Q_OBJECT

public:
	static constexpr const char *ClassIdentifier = "modifier";
	static constexpr const char *DatabaseFolder = "modifiers";

	static void InitializeAll()
	{
		DataType<IdentifiableModifier>::InitializeAll();

		//store pointers to the hardcoded modifiers
		IdentifiableModifier::OverpopulationModifier = IdentifiableModifier::Get("overpopulation");
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
	void Remove(T *scope) const //to prevent ambiguity for name lookup between ModifierBase::Remove and DataType::Remove
	{
		this->ModifierBase::Remove(scope);
	}
};

}
