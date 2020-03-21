#pragma once

namespace metternich {

class technology;

struct technology_compare
{
	bool operator()(technology *a, technology *b) const;
};

}
