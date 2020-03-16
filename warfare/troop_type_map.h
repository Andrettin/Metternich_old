#pragma once

#include <map>

namespace metternich {

class troop_type;

struct troop_type_compare
{
	bool operator()(troop_type *a, troop_type *b) const;
};

template <typename T>
using troop_type_map = std::map<troop_type *, T, troop_type_compare>;

}
