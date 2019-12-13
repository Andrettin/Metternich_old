#pragma once

#include <memory>
#include <vector>

namespace metternich {

class character;
class gsml_data;
class gsml_property;

template <typename T>
class condition;

template <typename T>
class event_option;

template <typename T>
class scoped_event_base
{
public:
	scoped_event_base();
	virtual ~scoped_event_base();

	void process_gsml_scope(const gsml_data &scope);
	bool check_conditions(const T *scope) const;
	void pick_option(T *scope) const;

private:
	std::unique_ptr<condition<T>> conditions;
	std::vector<std::unique_ptr<event_option<T>>> options;
};

extern template class scoped_event_base<character>;

}
