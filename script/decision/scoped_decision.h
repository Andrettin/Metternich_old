#pragma once

#include <memory>

namespace metternich {

class character;
class gsml_data;
class holding;

template <typename T>
class condition;

template <typename T>
class effect_list;

template <typename T>
class scoped_decision
{
public:
	scoped_decision();
	virtual ~scoped_decision();

	void process_gsml_scope(const gsml_data &scope);

	bool check_preconditions(const T *scope) const;
	bool check_preconditions(const T *scope, const character *source) const;
	bool check_conditions(const T *scope) const;
	bool check_conditions(const T *scope, const character *source) const;
	bool check_source_preconditions(const character *source) const;
	bool check_source_conditions(const character *source) const;
	void do_effects(T *scope, character *source) const;
	std::string get_effects_string(const T *scope) const;

private:
	std::unique_ptr<condition<T>> preconditions;
	std::unique_ptr<condition<T>> conditions;
	std::unique_ptr<condition<character>> source_preconditions;
	std::unique_ptr<condition<character>> source_conditions;
	std::unique_ptr<effect_list<T>> effects;
};

extern template class scoped_decision<holding>;

}