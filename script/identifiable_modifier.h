#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"
#include "script/modifier_base.h"

namespace metternich {

class GSMLProperty;

/**
**	@brief	An identifiable modifier
*/
class IdentifiableModifier : public DataEntry, public DataType<IdentifiableModifier>, public ModifierBase
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
	IdentifiableModifier(const std::string &identifier) : DataEntry(identifier) {}

	virtual void ProcessGSMLProperty(const GSMLProperty &property) override;

	template <typename T>
	void Remove(T *scope) const //to prevent ambiguity for name lookup between ModifierBase::Remove and DataType::Remove
	{
		this->ModifierBase::Remove(scope);
	}
};

}
