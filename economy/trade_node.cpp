#include "economy/trade_node.h"

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

world *trade_node::get_world() const
{
	if (this->get_center_of_trade() != nullptr) {
		return this->get_center_of_trade()->get_world();
	}

	return nullptr;
}

}
