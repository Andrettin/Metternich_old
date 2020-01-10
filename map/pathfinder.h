#pragma once

#include <boost/graph/adjacency_list.hpp>

#include <set>

namespace metternich {

class province;

class pathfinder
{
	using cost = int;
	using graph = boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS, boost::no_property, boost::property<boost::edge_weight_t, cost>>;
	using vertex = graph::vertex_descriptor;
	using edge_descriptor = graph::edge_descriptor;

public:
	pathfinder(const std::set<province *> &provinces);

	const std::vector<province *> &get_provinces() const
	{
		return this->provinces;
	}

	province *get_province(const vertex v) const
	{
		return this->provinces[v];
	}

	size_t get_province_index(const province *province) const
	{
		return this->province_to_index.find(province)->second;
	}

	const graph &get_province_graph() const
	{
		return this->province_graph;
	}

private:
	std::vector<province *> provinces;
	std::map<const province *, size_t> province_to_index;
	graph province_graph;
};

}
