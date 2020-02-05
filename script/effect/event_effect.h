#pragma once

#include "script/context.h"
#include "script/effect/effect.h"
#include "script/event/character_event.h"
#include "util/string_util.h"

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

	virtual void do_assignment_effect(T *scope, const context &ctx) const override
	{
		context new_ctx;
		new_ctx.source_character = ctx.current_character;

		if constexpr (std::is_same_v<T, character>) {
			new_ctx.current_character = scope;
			const character_event *event = static_cast<const character_event *>(this->event);
			if (event->check_conditions(scope)) {
				event->do_event(scope, new_ctx);
			}
		}
	}

	virtual std::string get_assignment_string() const override
	{
		return "Receive the " + string::highlight(this->event->get_name()) + " event";
	}

private:
	const event *event = nullptr;
};

}
