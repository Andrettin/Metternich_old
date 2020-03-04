#include "map/pathfinder.h"

#include "defines.h"
#include "map/province.h"
#include "util/container_util.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/astar_search.hpp>

namespace metternich {

class pathfinder::impl
{
	using cost = int;
	using graph = boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS, boost::no_property, boost::property<boost::edge_weight_t, cost>>;
	using vertex = graph::vertex_descriptor;
	using edge = graph::edge_descriptor;

public:
	impl(const std::set<province *> &provinces);

	find_trade_path_result find_trade_path(const province *start_province, const province *goal_province) const;

private:
	size_t get_province_index(const province *province) const
	{
		return this->province_to_index.find(province)->second;
	}

	cost get_trade_cost(const edge e) const;

private:
	std::vector<province *> provinces;
	std::map<const province *, size_t> province_to_index;
	graph province_graph;
};

template <class graph, class cost>
class trade_path_heuristic final : public boost::astar_heuristic<graph, cost>
{
public:
	using vertex = typename boost::graph_traits<graph>::vertex_descriptor;

	trade_path_heuristic(vertex goal, const std::vector<province *> &provinces)
		: goal(goal), provinces(provinces)
	{}

	cost operator()(vertex v)
	{
		return this->provinces[v]->get_kilometers_distance_to(this->provinces[this->goal]) * 100 / province::base_distance * defines::get()->get_trade_cost_modifier_per_distance() / 100;
	}

private:
	vertex goal;
	const std::vector<province *> &provinces;
};

template <class vertex>
class astar_visitor final : public boost::default_astar_visitor
{
public:
	astar_visitor(vertex goal) : goal(goal)
	{}

	template <class graph>
	void examine_vertex(vertex v, graph &g) {
		Q_UNUSED(g)

		if (v == this->goal) {
			throw found_goal();
		}
	}

private:
	vertex goal;
};

struct found_goal
{
};

pathfinder::pathfinder(const std::set<province *> &provinces)
{
	this->implementation = std::make_unique<impl>(provinces);
}

pathfinder::~pathfinder()
{
}

find_trade_path_result pathfinder::find_trade_path(const province *start_province, const province *goal_province) const
{
	return this->implementation->find_trade_path(start_province, goal_province);
}

pathfinder::impl::impl(const std::set<province *> &provinces)
	: provinces(container::to_vector(provinces)), province_graph(provinces.size())
{
	for (size_t i = 0; i < this->provinces.size(); ++i) {
		const province *province = this->provinces[i];
		this->province_to_index[province] = i;
	}

	for (province *province : this->provinces) {
		for (metternich::province *border_province : province->get_border_provinces()) {
			edge e;
			bool inserted;
			boost::tie(e, inserted) = boost::add_edge(this->province_to_index[province], this->province_to_index[border_province], this->province_graph);
		}
	}
}

find_trade_path_result pathfinder::impl::find_trade_path(const province *start_province, const province *goal_province) const
{
	vertex start = this->get_province_index(start_province);
	vertex goal = this->get_province_index(goal_province);

	std::vector<vertex> vertex_predecessors(boost::num_vertices(this->province_graph));
	std::vector<cost> vertex_costs(boost::num_vertices(this->province_graph));

	auto weight_function = boost::make_function_property_map<edge, cost>([this](edge e) {
		return this->get_trade_cost(e);
	});

	try {
		astar_search_tree(this->province_graph, start, trade_path_heuristic<graph, cost>(goal, this->provinces),
			weight_map(weight_function).
			predecessor_map(make_iterator_property_map(vertex_predecessors.begin(), get(boost::vertex_index, this->province_graph))).
			distance_map(make_iterator_property_map(vertex_costs.begin(), get(boost::vertex_index, this->province_graph))).
			visitor(astar_visitor<vertex>(goal)));
	} catch (found_goal) {
		find_trade_path_result result(true);
		for (vertex v = goal;; v = vertex_predecessors[v]) {
			result.path.push_back(this->provinces[v]);
			if (vertex_predecessors[v] == v) {
				break;
			}
		}
		std::reverse(result.path.begin(), result.path.end());
		result.trade_cost = vertex_costs[goal];
		return result;
	}

	return find_trade_path_result(false);
}

pathfinder::impl::cost pathfinder::impl::get_trade_cost(const edge e) const
{
	const province *source_province = this->provinces[e.m_source];
	const province *target_province = this->provinces[e.m_target];

	int trade_cost = source_province->get_kilometers_distance_to(target_province) * 100 / province::base_distance * defines::get()->get_trade_cost_modifier_per_distance() / 100;

	if (source_province->is_water() != target_province->is_water()) {
		trade_cost += defines::get()->get_base_port_trade_cost_modifier();
	}

	return trade_cost;
}

}
