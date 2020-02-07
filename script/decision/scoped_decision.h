#pragma once

#include <memory>

namespace metternich {

class character;
class gsml_data;
class gsml_property;
class holding;

template <typename T>
class and_condition;

template <typename T>
class chance_factor;

template <typename T>
class decision_filter;

template <typename T>
class effect_list;

template <typename T>
class scoped_decision
{
public:
	scoped_decision();
	virtual ~scoped_decision();

	void initialize();

	void process_gsml_property(const gsml_property &property);
	void process_gsml_scope(const gsml_data &scope);

	bool check_filter(const T *scope, const character *source) const;
	bool check_preconditions(const T *scope) const;
	bool check_preconditions(const T *scope, const character *source) const;
	bool check_conditions(const T *scope) const;
	bool check_conditions(const T *scope, const character *source) const;
	bool check_source_preconditions(const character *source) const;
	bool check_source_conditions(const character *source) const;
	void do_effects(T *scope, character *source) const;
	QString get_string(const T *scope, character *source) const;
	int calculate_ai_chance(const T *scope, character *source) const;

private:
	decision_filter<T> *filter = nullptr;
	std::unique_ptr<and_condition<T>> preconditions;
	std::unique_ptr<and_condition<T>> conditions;
	std::unique_ptr<and_condition<character>> source_preconditions;
	std::unique_ptr<and_condition<character>> source_conditions;
	std::unique_ptr<effect_list<T>> effects;
	std::unique_ptr<chance_factor<T>> ai_chance; //the chance of the AI doing the decision each time it is checked
};

extern template class scoped_decision<holding>;

}
