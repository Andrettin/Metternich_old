#pragma once

namespace Metternich {

class GSMLData;
class GSMLProperty;
class Holding;
class Province;

/**
**	@brief	A scripted condition
*/
class Condition
{
public:
	static std::unique_ptr<Condition> FromGSMLProperty(const GSMLProperty &property);
	static std::unique_ptr<Condition> FromGSMLScope(const GSMLData &scope);

	virtual ~Condition() {}

	virtual void ProcessGSMLProperty(const GSMLProperty &property);
	virtual void ProcessGSMLScope(const GSMLData &scope);

	virtual const std::string &GetIdentifier() const = 0;

	virtual bool Check(const Province *province) const
	{
		Q_UNUSED(province);
		throw std::runtime_error("Invalid condition for province: \"" + this->GetIdentifier() + "\".");
	}

	virtual bool Check(const Holding *holding) const
	{
		Q_UNUSED(holding);
		throw std::runtime_error("Invalid condition for holding: \"" + this->GetIdentifier() + "\".");
	}
};

}
