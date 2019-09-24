#pragma once

namespace metternich {

class gsml_data;
class gsml_property;
class holding;
class Province;

/**
**	@brief	A scripted condition
*/
class Condition
{
public:
	static std::unique_ptr<Condition> FromGSMLProperty(const gsml_property &property);
	static std::unique_ptr<Condition> FromGSMLScope(const gsml_data &scope);

	virtual ~Condition() {}

	virtual void process_gsml_property(const gsml_property &property);
	virtual void process_gsml_scope(const gsml_data &scope);

	virtual const std::string &get_identifier() const = 0;

	virtual bool Check(const Province *province) const
	{
		Q_UNUSED(province);
		throw std::runtime_error("Invalid condition for province: \"" + this->get_identifier() + "\".");
	}

	virtual bool Check(const holding *holding) const
	{
		Q_UNUSED(holding);
		throw std::runtime_error("Invalid condition for holding: \"" + this->get_identifier() + "\".");
	}
};

}
