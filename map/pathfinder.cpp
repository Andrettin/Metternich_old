#include "map/pathfinder.h"

#include "defines.h"
#include "map/province.h"
#include "util/container_util.h"
#include "util/map_util.h"
#include "util/point_container.h"
#include "util/point_util.h"
#include "util/set_util.h"

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
	find_province_pos_path_result find_province_pos_path(const province *start_province, const province *goal_province) const;

	void add_province_pos_path(const std::vector<QPoint> &pos_list)
	{
		for (size_t i = 1; i < pos_list.size(); ++i) {
			const QPoint &pos = pos_list[i];
			const QPoint &previous_pos = pos_list[i - 1];
			this->province_pos_connections[previous_pos].push_back(pos);
			this->province_pos_connections[pos].push_back(previous_pos);
		}
	}

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

	point_map<std::vector<QPoint>> province_pos_connections;
};

template <class graph, class cost>
class trade_path_heuristic : public boost::astar_heuristic<graph, cost>
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

template <class graph, class cost>
class province_pos_path_heuristic : public boost::astar_heuristic<graph, cost>
{
public:
	using vertex = typename boost::graph_traits<graph>::vertex_descriptor;

	province_pos_path_heuristic(vertex goal, const std::vector<QPoint> &pos_list)
		: goal(goal), pos_list(pos_list)
	{}

	cost operator()(vertex v)
	{
		return point::distance_to(this->pos_list[v], this->pos_list[this->goal]);
	}

private:
	vertex goal;
	const std::vector<QPoint> &pos_list;
};

template <class vertex>
class astar_visitor : public boost::default_astar_visitor
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

find_province_pos_path_result pathfinder::find_province_pos_path(const province *start_province, const province *goal_province) const
{
	return this->implementation->find_province_pos_path(start_province, goal_province);
}

void pathfinder::add_province_pos_path(const std::vector<QPoint> &pos_list)
{
	this->implementation->add_province_pos_path(pos_list);
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

find_province_pos_path_result pathfinder::impl::find_province_pos_path(const province *start_province, const province *goal_province) const
{
	point_set pos_set = start_province->get_path_pos_list();
	set::merge(pos_set, goal_province->get_path_pos_list());

	if (pos_set.empty()) {
		return find_province_pos_path_result(false);
	}

	point_map<std::vector<QPoint>> pos_connections;

	if (this->province_pos_connections.empty()) {
		throw std::runtime_error("No province position connections are present for the pathfinder, despite provinces having path positions.");
	}

	for (const QPoint &pos : pos_set) {
		auto find_iterator = this->province_pos_connections.find(pos);
		if (find_iterator == this->province_pos_connections.end()) {
			continue;
		}

		for (const QPoint &connected_pos : find_iterator->second) {
			if (!pos_set.contains(connected_pos)) {
				continue;
			}

			if (!start_province->is_valid_line_to(pos, connected_pos, goal_province)) {
				continue;
			}

			pos_connections[pos].push_back(connected_pos);
		}
	}

	const std::vector<QPoint> pos_list = map::get_key_vector(pos_connections);

	point_map<size_t> pos_to_index;

	for (size_t i = 0; i < pos_list.size(); ++i) {
		const QPoint &pos = pos_list[i];
		pos_to_index[pos] = i;
	}

	const QPoint start_pos = point::get_nearest_point(start_province->get_main_pos(), pos_list);
	const QPoint goal_pos = point::get_nearest_point(goal_province->get_main_pos(), pos_list);
	vertex start = pos_to_index.find(start_pos)->second;
	vertex goal = pos_to_index.find(goal_pos)->second;

	graph province_pos_graph(pos_list.size());

	for (const QPoint &pos : pos_list) {
		auto find_iterator = pos_connections.find(pos);
		if (find_iterator == pos_connections.end() || find_iterator->second.empty()) {
			throw std::runtime_error("No connection found for province pos (" + std::to_string(pos.x()) + ", " + std::to_string(pos.y()) + ").");
		}

		bool added_edge = false;
		for (const QPoint &connected_pos : find_iterator->second) {
			edge e;
			bool inserted;
			boost::tie(e, inserted) = boost::add_edge(pos_to_index[pos], pos_to_index[connected_pos], province_pos_graph);
			added_edge = true;
		}

		if (!added_edge) {
			throw std::runtime_error("No edge added for province pos (" + std::to_string(pos.x()) + ", " + std::to_string(pos.y()) + ").");
		}
	}

	std::vector<vertex> vertex_predecessors(boost::num_vertices(province_pos_graph));
	std::vector<cost> vertex_costs(boost::num_vertices(province_pos_graph));

	auto weight_function = boost::make_function_property_map<edge, cost>([pos_list](edge e) {
		return point::distance_to(pos_list[e.m_source], pos_list[e.m_target]);
	});

	try {
		astar_search_tree(province_pos_graph, start, province_pos_path_heuristic<graph, cost>(goal, pos_list),
			weight_map(weight_function).
			predecessor_map(make_iterator_property_map(vertex_predecessors.begin(), get(boost::vertex_index, province_pos_graph))).
			distance_map(make_iterator_property_map(vertex_costs.begin(), get(boost::vertex_index, province_pos_graph))).
			visitor(astar_visitor<vertex>(goal)));
	} catch (found_goal) {
		find_province_pos_path_result result(true);
		for (vertex v = goal;; v = vertex_predecessors[v]) {
			result.path.push_back(pos_list[v]);
			if (vertex_predecessors[v] == v) {
				break;
			}
		}
		std::reverse(result.path.begin(), result.path.end());
		return result;
	}

	return find_province_pos_path_result(false);
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
