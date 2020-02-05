#pragma once

#include <memory>
#include <string>
#include <vector>

namespace metternich {

class character;
class gsml_data;
class gsml_property;
class holding;
class province;
struct context;

template <typename T>
class effect;

template <typename T>
class effect_list
{
public:
	effect_list();
	~effect_list();

	void process_gsml_property(const gsml_property &property);
	void process_gsml_scope(const gsml_data &scope);
	void do_effects(T *scope, const context &ctx) const;
	std::string get_effects_string(const T *scope, const context &ctx, const size_t indent = 0) const;

private:
	std::vector<std::unique_ptr<effect<T>>> effects;
};

extern template class effect_list<character>;
extern template class effect_list<holding>;
extern template class effect_list<province>;

}
