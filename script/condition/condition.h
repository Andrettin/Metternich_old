#pragma once

namespace metternich {

class gsml_data;
class gsml_property;
class holding;
class holding_slot;
class population_unit;
class province;

template <typename T>
class condition_check;

/**
**	@brief	A scripted condition
*/
template <typename T>
class condition
{
public:
	static std::unique_ptr<condition> from_gsml_property(const gsml_property &property);
	static std::unique_ptr<condition> from_gsml_scope(const gsml_data &scope);

	virtual ~condition() {}

	virtual void process_gsml_property(const gsml_property &property);
	virtual void process_gsml_scope(const gsml_data &scope);

	virtual const std::string &get_identifier() const = 0;

	virtual bool check(const T *scope) const = 0;

	virtual void bind_condition_check(condition_check<T> &check, const T *scope) const
	{
		Q_UNUSED(check)
		Q_UNUSED(scope)
	}
};

extern template class condition<holding>;
extern template class condition<holding_slot>;
extern template class condition<population_unit>;
extern template class condition<province>;

}
