#pragma once

namespace Metternich {

class GSMLData;
class GSMLProperty;
class Holding;
class Province;

/**
**	@brief	A modifier effect
*/
class ModifierEffect
{
public:
	static std::unique_ptr<ModifierEffect> FromGSMLProperty(const GSMLProperty &property);

	virtual ~ModifierEffect() {}

	virtual const std::string &GetIdentifier() const = 0;

	virtual void Apply(Province *province, const int change) const
	{
		Q_UNUSED(province);
		Q_UNUSED(change);
		throw std::runtime_error("Invalid modifier effect for province: \"" + this->GetIdentifier() + "\".");
	}

	virtual void Apply(Holding *holding, const int change) const
	{
		Q_UNUSED(holding);
		Q_UNUSED(change);
		throw std::runtime_error("Invalid modifier effect for holding: \"" + this->GetIdentifier() + "\".");
	}
};

}
