#pragma once

#include <memory>
#include <vector>

namespace metternich {

class character;
class gsml_data;
class gsml_property;
class holding;
class province;

template <typename T>
class modifier_effect;

/**
**	@brief	A modifier (i.e. a collection of modifier effects)
*/
template <typename T>
class modifier
{
public:
	modifier();
	virtual ~modifier();

	void process_gsml_property(const gsml_property &property);

	void process_gsml_scope(const gsml_data &scope)
	{
		Q_UNUSED(scope)
	}

	void apply(T *scope) const;
	void remove(T *scope) const;
	std::string get_string() const;

private:
	void add_modifier_effect(std::unique_ptr<modifier_effect<T>> &&modifier_effect);

private:
	std::vector<std::unique_ptr<modifier_effect<T>>> modifier_effects;
};

extern template class modifier<character>;
extern template class modifier<holding>;
extern template class modifier<province>;

}
