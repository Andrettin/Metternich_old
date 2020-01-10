#include "map/pathfinder.h"

#include "map/province.h"
#include "util/container_util.h"

namespace metternich {

pathfinder::pathfinder(const std::set<province *> &provinces)
	: provinces(container::to_vector(provinces)), province_graph(provinces.size())
{
	for (size_t i = 0; i < this->provinces.size(); ++i) {
		const province *province = this->provinces[i];
		this->province_to_index[province] = i;
	}

	for (province *province : this->provinces) {
		for (metternich::province *border_province : province->get_border_provinces()) {
			edge_descriptor e;
			bool inserted;
			boost::tie(e, inserted) = boost::add_edge(this->province_to_index[province], this->province_to_index[border_province], this->province_graph);
		}
	}
}

}
