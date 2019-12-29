#pragma once

#include <set>

namespace metternich {

class technology;

struct technology_compare
{
	bool operator()(technology *a, technology *b) const;
};

using technology_set = std::set<technology *, technology_compare>;

}
