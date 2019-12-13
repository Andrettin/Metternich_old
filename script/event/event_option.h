#pragma once

#include <memory>
#include <vector>

namespace metternich {

class character;
class gsml_data;
class gsml_property;

template <typename T>
class effect;

template <typename T>
class event_option
{
public:
	event_option();
	virtual ~event_option();

	void process_gsml_property(const gsml_property &property);
	void process_gsml_scope(const gsml_data &scope);

	std::string get_name() const;
	void do_effects(T *scope) const;

private:
	std::string name_tag;
	std::vector<std::unique_ptr<effect<T>>> effects;
};

extern template class event_option<character>;

}
