#pragma once

#include <memory>
#include <vector>

namespace metternich {

class character;
class gsml_data;
class gsml_property;

template <typename T>
class chance_factor;

template <typename T>
class effect;

template <typename T>
class event_option
{
public:
	event_option();
	~event_option();

	void process_gsml_property(const gsml_property &property);
	void process_gsml_scope(const gsml_data &scope);

	std::string get_name() const;
	std::string get_effects_string(const T *scope) const;

	const chance_factor<T> *get_ai_chance_factor() const
	{
		return this->ai_chance.get();
	}

	void do_effects(T *scope) const;

private:
	std::string name_tag;
	std::unique_ptr<chance_factor<T>> ai_chance; //the chance of the option being picked by the AI
	std::vector<std::unique_ptr<effect<T>>> effects;
};

extern template class event_option<character>;

}
