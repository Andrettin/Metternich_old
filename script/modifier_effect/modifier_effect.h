#pragma once

namespace metternich {

class character;
class gsml_property;
class holding;
class province;

template <typename T>
class modifier_effect
{
public:
	static std::unique_ptr<modifier_effect> from_gsml_property(const gsml_property &property);

	virtual ~modifier_effect() {}

	virtual const std::string &get_identifier() const = 0;
	virtual void apply(T *scope, const int change) const = 0;
	virtual std::string get_string() const = 0;
};

extern template class modifier_effect<character>;
extern template class modifier_effect<holding>;
extern template class modifier_effect<province>;

}
