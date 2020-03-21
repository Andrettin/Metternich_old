#pragma once

namespace metternich {

class technology_area;

struct technology_area_compare
{
	bool operator()(technology_area *a, technology_area *b) const;
};

}
