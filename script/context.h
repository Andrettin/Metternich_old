#pragma once

#include <type_traits>

namespace metternich {

class character;

//script context for e.g. events and decisions
template <bool read_only>
struct context_base
{
	using character_ptr = std::conditional_t<read_only, const character *, character *>;

	character_ptr source_character = nullptr;
	character_ptr current_character = nullptr;
};

struct context : context_base<false>
{
};

struct read_only_context : context_base<true>
{
public:
	read_only_context()
	{
	}

	read_only_context(const context &ctx)
	{
		this->source_character = ctx.source_character;
		this->current_character = ctx.current_character;
	}
};

}
