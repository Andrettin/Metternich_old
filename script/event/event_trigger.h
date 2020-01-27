#pragma once

#include "database/identifiable_type.h"

namespace metternich {

class character;

template <typename T>
class scoped_event_base;

class event_trigger_base
{
public:
	event_trigger_base(const std::string &identifier) : identifier(identifier)
	{
	}

	const std::string &get_identifier() const
	{
		return this->identifier;
	}

private:
	std::string identifier;
};

template <typename T>
class event_trigger : public event_trigger_base, public identifiable_type<event_trigger<T>>
{
public:
	static constexpr const char *class_identifier = "event_trigger";

	event_trigger(const std::string &identifier) : event_trigger_base(identifier)
	{
	}

	const std::vector<const scoped_event_base<T> *> &get_events() const
	{
		return this->events;
	}

	void add_event(const scoped_event_base<T> *event)
	{
		this->events.push_back(event);
	}

private:
	std::vector<const scoped_event_base<T> *> events;
};

class character_event_trigger : public event_trigger<character>
{
public:
	static inline const event_trigger<character> *monthly_pulse = event_trigger::add("monthly_pulse");
	static inline const event_trigger<character> *yearly_pulse = event_trigger::add("yearly_pulse");
};

}
