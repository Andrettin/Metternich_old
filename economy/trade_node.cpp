#include "economy/trade_node.h"

#include "game/game.h"
#include "map/province.h"
#include "translator.h"

namespace metternich {

void trade_node::check() const
{
	if (!this->get_color().isValid()) {
		throw std::runtime_error("Trade node \"" + this->get_identifier() + "\" has no valid color.");
	}

	if (this->get_center_of_trade() == nullptr) {
		throw std::runtime_error("Trade node \"" + this->get_identifier() + "\" has no center of trade province.");
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
		trade_node::active_trade_nodes.insert(this);
	} else {
		trade_node::active_trade_nodes.erase(this);
	}

	if (game::get()->is_running()) {
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
	}
}

void trade_node::set_major(const bool major)
{
	if (major == this->is_major()) {
		return;
	}

	this->major = major;

	if (major) {
		trade_node::major_trade_nodes.insert(this);
	} else {
		trade_node::major_trade_nodes.erase(this);
	}

	if (game::get()->is_running()) {
		//recalculate the trade area of all nodes if this trade node is becoming major, or of its dependent nodes if becoming minor
		//recalculate the trade area of all provinces
		if (major) {
			this->calculate_trade_area();

			for (metternich::trade_node *node : trade_node::get_all_active()) {
				if (node->is_major()) {
					continue;
				}

				node->get_center_of_trade()->set_trade_node_recalculation_needed(true, false);
			}
		} else {
			for (metternich::trade_node *node : this->get_trade_nodes()) {
				if (node == this) {
					continue;
				}

				node->get_center_of_trade()->calculate_trade_node();
			}
		}
	}
}

void trade_node::set_trade_area(trade_node *trade_area)
{
	if (trade_area == this->get_trade_area()) {
		return;
	}

	this->trade_area = trade_area;
	emit trade_area_changed();

	for (province *node_province : this->get_provinces()) {
		emit node_province->trade_area_changed();
	}
}

void trade_node::calculate_trade_area()
{
	if (this->is_major()) {
		//if this is a major trade node, then it is necessarily a part of its own area
		this->set_trade_area(this);
		return;
	}

	if (this->get_center_of_trade()->get_owner() == nullptr) {
		//provinces without an owner don't get assigned to any trade node
		this->set_trade_area(nullptr);
		return;
	}

	province *center_of_trade = this->get_center_of_trade();
	metternich::trade_node *best_area = center_of_trade->get_best_trade_node_from_list(trade_node::major_trade_nodes);

	this->set_trade_area(best_area);
}

world *trade_node::get_world() const
{
	if (this->get_center_of_trade() != nullptr) {
		return this->get_center_of_trade()->get_world();
	}

	return nullptr;
}

}
