#pragma once

namespace metternich {

class gsml_property;
class holding;
class province;

/**
**	@brief	A modifier effect
*/
class modifier_effect
{
public:
	static std::unique_ptr<modifier_effect> from_gsml_property(const gsml_property &property);

	virtual ~modifier_effect() {}

	virtual const std::string &get_identifier() const = 0;

	virtual void apply(province *province, const int change) const
	{
		Q_UNUSED(province)
		Q_UNUSED(change)
		throw std::runtime_error("Invalid modifier effect for province: \"" + this->get_identifier() + "\".");
	}

	virtual void apply(holding *holding, const int change) const
	{
		Q_UNUSED(holding)
		Q_UNUSED(change)
		throw std::runtime_error("Invalid modifier effect for holding: \"" + this->get_identifier() + "\".");
	}
};

}
