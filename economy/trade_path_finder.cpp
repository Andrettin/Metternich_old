#include "economy/trade_path_finder.h"

#include "defines.h"
#include "map/province.h"
#include "map/world.h"
#include "util/container_util.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/astar_search.hpp>

namespace metternich {

using cost_type = int;

template <class graph_type, class cost_type>
class distance_heuristic : public boost::astar_heuristic<graph_type, cost_type>
{
public:
	using vertex_type = typename boost::graph_traits<graph_type>::vertex_descriptor;

	distance_heuristic(vertex_type goal, const std::vector<province *> &provinces)
		: goal(goal), provinces(provinces)
	{}

	cost_type operator()(vertex_type vertex)
	{
		return this->provinces[vertex]->get_kilometers_distance_to(this->provinces[this->goal]) * 100 / province::base_distance * defines::get()->get_trade_cost_modifier_per_distance() / 100;
	}

private:
	vertex_type goal;
	const std::vector<province *> &provinces;
};

struct found_goal
{
};

template <class vertex_type>
class astar_goal_visitor : public boost::default_astar_visitor
{
public:
	astar_goal_visitor(vertex_type goal) : goal(goal)
	{}

	template <class graph_type>
	void examine_vertex(vertex_type vertex, graph_type &graph) {
		Q_UNUSED(graph)

		if (vertex == this->goal) {
			throw found_goal();
		}
	}

private:
	vertex_type goal;
};

bool trade_path_finder::find_path(const province *start_province, const province *goal_province)
{
	this->path.clear();

	using graph_type = boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS, boost::no_property, boost::property<boost::edge_weight_t, cost_type>>;
	using vertex_type = graph_type::vertex_descriptor;
	using edge_descriptor = graph_type::edge_descriptor;

	std::vector<province *> provinces = container::to_vector(start_province->get_world()->get_provinces());
	std::map<const province *, size_t> province_to_index;

	for (size_t i = 0; i < provinces.size(); ++i) {
		const province *province = provinces[i];
		province_to_index[province] = i;
	}

	graph_type graph(provinces.size());

	for (province *province : provinces) {
		for (metternich::province *border_province : province->get_border_provinces()) {
			edge_descriptor e;
			bool inserted;
			boost::tie(e, inserted) = boost::add_edge(province_to_index[province], province_to_index[border_province], graph);
		}
	}

	auto weight_function = boost::make_function_property_map<edge_descriptor, cost_type>([&provinces](edge_descriptor e) {
		const province *source_province = provinces[e.m_source];
		const province *target_province = provinces[e.m_target];
		return source_province->get_kilometers_distance_to(target_province) * 100 / province::base_distance * defines::get()->get_trade_cost_modifier_per_distance() / 100;
	});

	vertex_type start = province_to_index[start_province];
	vertex_type goal = province_to_index[goal_province];

	std::vector<vertex_type> vertex_predecessors(boost::num_vertices(graph));
	std::vector<cost_type> vertex_costs(boost::num_vertices(graph));
	try {
		astar_search_tree(graph, start, distance_heuristic<graph_type, cost_type>(goal, provinces),
			weight_map(weight_function).
			predecessor_map(make_iterator_property_map(vertex_predecessors.begin(), get(boost::vertex_index, graph))).
			distance_map(make_iterator_property_map(vertex_costs.begin(), get(boost::vertex_index, graph))).
			visitor(astar_goal_visitor<vertex_type>(goal)));
	} catch (found_goal) {
		for (vertex_type v = goal;; v = vertex_predecessors[v]) {
			this->path.push_back(provinces[v]);
			if (vertex_predecessors[v] == v) {
				break;
			}
		}
		std::reverse(this->path.begin(), this->path.end());
		this->trade_cost = vertex_costs[goal];
		return true;
	}

	return false;
}

}
