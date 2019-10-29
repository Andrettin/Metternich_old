#pragma once

namespace metternich {

class gsml_data;
class gsml_property;
class holding;
class holding_slot;
class province;

/**
**	@brief	A scripted condition
*/
class condition
{
public:
	static std::unique_ptr<condition> from_gsml_property(const gsml_property &property);
	static std::unique_ptr<condition> from_gsml_scope(const gsml_data &scope);

	virtual ~condition() {}

	virtual void process_gsml_property(const gsml_property &property);
	virtual void process_gsml_scope(const gsml_data &scope);

	virtual const std::string &get_identifier() const = 0;

	virtual bool check(const province *province) const
	{
		Q_UNUSED(province)
		throw std::runtime_error("Invalid condition for province: \"" + this->get_identifier() + "\".");
	}

	virtual bool check(const holding *holding) const
	{
		Q_UNUSED(holding)
		throw std::runtime_error("Invalid condition for holding: \"" + this->get_identifier() + "\".");
	}

	virtual bool check(const holding_slot *holding_slot) const
	{
		Q_UNUSED(holding_slot)
		throw std::runtime_error("Invalid condition for holding slot: \"" + this->get_identifier() + "\".");
	}
};

}
