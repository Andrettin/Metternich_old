#pragma once

#include "script/effect/effect.h"
#include "script/event/character_event.h"

#include <type_traits>

namespace metternich {

class character;
class event;

template <typename T>
class event_effect : public effect<T>
{
public:
	event_effect(const std::string &event_identifier, const gsml_operator effect_operator)
		: effect<T>(effect_operator)
	{
		if constexpr (std::is_same_v<T, character>) {
			this->event = character_event::get(event_identifier);
		}
	}

	virtual const std::string &get_identifier() const override
	{
		static const std::string identifier = "event";
		return identifier;
	}

	virtual void do_assignment_effect(T *scope) const override
	{
		if constexpr (std::is_same_v<T, character>) {
			if (static_cast<const character_event *>(this->event)->check_conditions(scope)) {
				static_cast<const character_event *>(this->event)->do_event(scope);
			}
		}
	}

	virtual bool is_hidden() const override
	{
		return true;
	}

private:
	const event *event = nullptr;
};

}
