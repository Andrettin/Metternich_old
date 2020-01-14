#include "economy/trade_node.h"

#include "history/history.h"
#include "map/pathfinder.h"
#include "map/province.h"
#include "map/world.h"
#include "translator.h"
#include "util/map_util.h"

namespace metternich {

void trade_node::initialize()
{
	if (this->get_center_of_trade() == nullptr) {
		throw std::runtime_error("Trade node \"" + this->get_identifier() + "\" has no center of trade province.");
	}

	this->get_world()->add_trade_node(this);
}

void trade_node::check() const
{
	if (!this->get_color().isValid()) {
		throw std::runtime_error("Trade node \"" + this->get_identifier() + "\" has no valid color.");
	}

	if (this->get_center_of_trade()->get_trade_node() == nullptr) {
		throw std::runtime_error("The center of trade province for trade node \"" + this->get_identifier() + "\" has no trade node set for it.");
	}

	if (this->get_center_of_trade()->get_trade_node() != this) {
		throw std::runtime_error("The center of trade province for trade node \"" + this->get_identifier() + "\" has another trade node set for it instead.");
	}
}

std::string trade_node::get_name() const
{
	return translator::get()->translate(this->get_identifier_with_aliases(), this->get_center_of_trade()->get_tag_suffix_list_with_fallbacks());
}

void trade_node::set_center_of_trade(province *province)
{
	if (province == this->get_center_of_trade()) {
		return;
	}

	this->center_of_trade = province;
	emit center_of_trade_changed();

	province->set_trade_node(this);
}

void trade_node::set_active(const bool active)
{
	if (active == this->is_active()) {
		return;
	}

	this->active = active;

	if (active) {
		this->get_world()->add_active_trade_node(this);
	} else {
		this->get_world()->remove_active_trade_node(this);
	}

	emit active_changed();

	if (!history::get()->is_loading()) {
		//recalculate the trade node of all provinces if this trade node is becoming active, or of its dependent provinces if becoming inactive
		if (active) {
			for (province *province : province::get_all()) {
				if (province->is_center_of_trade() || !province->get_owner()) {
					continue;
				}

				province->set_trade_node_recalculation_needed(true);
			}
		} else {
			for (province *province : this->get_provinces()) {
				if (province == this->get_center_of_trade()) {
					continue;
				}

				province->calculate_trade_node();
			}
		}

		this->calculate_trade_paths();

		for (trade_node *node : this->get_world()->get_active_trade_nodes()) {
			if (node == this) {
				continue;
			}

			node->calculate_trade_path(this);
		}
	}
}

world *trade_node::get_world() const
{
	if (this->get_center_of_trade() != nullptr) {
		return this->get_center_of_trade()->get_world();
	}

	return nullptr;
}

void trade_node::set_trade_path(const trade_node *other_node, const std::vector<province *> &path, const bool notify)
{
	if (path == this->get_trade_path(other_node)) {
		return;
	}

	if (path.empty()) {
		this->trade_paths.erase(other_node);
	} else {
		this->trade_paths[other_node] = path;
	}

	if (notify) {
		emit trade_paths_changed();
	}
}

void trade_node::calculate_trade_paths()
{
	if (!this->is_active()) {
		this->clear_trade_paths();
		this->trade_costs.clear();
		return;
	}

	for (const trade_node *node : this->get_world()->get_active_trade_nodes()) {
		if (node == this) {
			continue;
		}

		this->calculate_trade_path(node, false);
	}

	emit trade_paths_changed();
}

void trade_node::calculate_trade_path(const trade_node *other_node, const bool notify)
{
	if (!other_node->is_active()) {
		this->set_trade_path(other_node, trade_node::empty_path, notify);
		this->trade_costs.erase(other_node);
		return;
	}

	//calculate the trade path with another node
	const pathfinder *pathfinder = this->get_world()->get_pathfinder();
	const find_trade_path_result result = pathfinder->find_trade_path(this->get_center_of_trade(), other_node->get_center_of_trade());

	if (!result.success) {
		return;
	}

	this->set_trade_path(other_node, result.path, notify);
	this->trade_costs[other_node] = result.trade_cost;
}

void trade_node::clear_trade_paths()
{
	const std::set<const trade_node *> path_nodes = map::get_keys(this->trade_paths);

	for (const trade_node *node : path_nodes) {
		this->set_trade_path(node, trade_node::empty_path, false);
	}

	emit trade_paths_changed();
}

}
