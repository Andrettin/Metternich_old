#pragma once

namespace metternich {

class character;

//script context for e.g. events and decisions
struct context
{
	character *source_character = nullptr;
	character *current_character = nullptr;
};

}
