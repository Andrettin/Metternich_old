#include "economy/trade_node.h"

#include "economy/trade_route.h"
#include "history/history.h"
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
	}

	//update the activity of trade routes which pass through this node
	for (trade_route *route : this->trade_routes) {
		if (active) {
			route->calculate_active();
		} else {
			route->set_active(false);
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

}
