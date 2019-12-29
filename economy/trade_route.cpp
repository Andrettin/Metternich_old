#include "economy/trade_route.h"

#include "economy/trade_node.h"
#include "map/province.h"
#include "map/world.h"
#include "util/container_util.h"

namespace metternich {

void trade_route::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "path") {
		for (const std::string &province_identifier : scope.get_values()) {
			province *path_province = province::get(province_identifier);
			this->path.push_back(path_province);
		}
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

void trade_route::initialize()
{
	if (this->path.empty()) {
		throw std::runtime_error("Trade route \"" + this->get_identifier() + "\" has no path.");
	}

	if (this->path.size() == 1) {
		throw std::runtime_error("The path of trade route \"" + this->get_identifier() + "\" consists of only one province.");
	}

	for (province *path_province : this->path) {
		path_province->add_trade_route(this);
	}

	province *start_province = this->path.front();
	province *end_province = this->path.back();
	start_province->get_trade_node()->add_trade_route(this);
	start_province->get_world()->add_trade_route(this);
	end_province->get_trade_node()->add_trade_route(this);
}

void trade_route::check() const
{
	const province *start_province = this->path.front();

	if (!start_province->is_center_of_trade()) {
		throw std::runtime_error("The path of trade route \"" + this->get_identifier() + "\" does not start with a center of trade.");
	}

	const province *end_province = this->path.back();

	if (!end_province->is_center_of_trade()) {
		throw std::runtime_error("The path of trade route \"" + this->get_identifier() + "\" does not end with a center of trade.");
	}
}

QVariantList trade_route::get_path_qvariant_list() const
{
	return container::to_qvariant_list(this->path);
}

}
