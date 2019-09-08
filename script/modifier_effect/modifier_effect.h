#pragma once

namespace metternich {

class gsml_property;
class holding;
class Province;

/**
**	@brief	A modifier effect
*/
class ModifierEffect
{
public:
	static std::unique_ptr<ModifierEffect> FromGSMLProperty(const gsml_property &property);

	virtual ~ModifierEffect() {}

	virtual const std::string &GetIdentifier() const = 0;

	virtual void Apply(Province *province, const int change) const
	{
		Q_UNUSED(province);
		Q_UNUSED(change);
		throw std::runtime_error("Invalid modifier effect for province: \"" + this->GetIdentifier() + "\".");
	}

	virtual void Apply(holding *holding, const int change) const
	{
		Q_UNUSED(holding);
		Q_UNUSED(change);
		throw std::runtime_error("Invalid modifier effect for holding: \"" + this->GetIdentifier() + "\".");
	}
};

}
