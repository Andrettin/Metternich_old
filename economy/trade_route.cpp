#include "economy/trade_route.h"

#include "map/province.h"
#include "map/world.h"
#include "util/container_util.h"
#include "util/map_util.h"

namespace metternich {

void trade_route::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "path") {
		this->clear_path();

		std::vector<province *> path;

		for (const std::string &province_identifier : scope.get_values()) {
			province *path_province = province::get(province_identifier);
			path.push_back(path_province);
		}

		this->add_path_branch(path);

	} else if (tag == "path_branches") {
		this->clear_path();

		for (const gsml_data &path_branch_data : scope.get_children()) {
			std::vector<province *> path_branch;

			for (const std::string &province_identifier : path_branch_data.get_values()) {
				province *path_province = province::get(province_identifier);
				path_branch.push_back(path_province);
			}

			this->add_path_branch(path_branch);
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

	for (const auto &kv_pair : this->path) {
		province *path_province = kv_pair.first;
		if (path_province == nullptr) {
			throw std::runtime_error("The path of trade route \"" + this->get_identifier() + "\" contains a null province.");
		}

		path_province->add_trade_route(this);

		trade_route_path_element *path_element = kv_pair.second.get();
		if (path_element->get_previous().empty() || path_element->get_next().empty()) {
			this->path_endpoints.insert(path_province);
		}
	}

	const province *endpoint_province = *this->path_endpoints.begin();

	if (this->get_world() == nullptr) {
		this->set_world(endpoint_province->get_world());
	}

	if (!this->get_rect().isValid()) {
		QPoint top_left(-1, -1);
		QPoint bottom_right(-1, -1);

		for (const auto &kv_pair : this->path) {
			const province *path_province = kv_pair.first;
			const QPoint province_pos = path_province->get_center_pos();

			if (top_left.x() == -1 || province_pos.x() < top_left.x()) {
				top_left.setX(province_pos.x());
			}

			if (top_left.y() == -1 || province_pos.y() < top_left.y()) {
				top_left.setY(province_pos.y());
			}

			if (bottom_right.x() == -1 || province_pos.x() > bottom_right.x()) {
				bottom_right.setX(province_pos.x());
			}

			if (bottom_right.y() == -1 || province_pos.y() > bottom_right.y()) {
				bottom_right.setY(province_pos.y());
			}
		}

		this->rect = QRect(top_left, bottom_right);
	}
}

void trade_route::set_world(metternich::world *world)
{
	if (world == this->get_world()) {
		return;
	}

	this->world = world;
	world->add_trade_route(this);
}

QVariantList trade_route::get_path_qvariant_list() const
{
	return container::to_qvariant_list(map::get_keys(this->path));
}

void trade_route::add_path_branch(const std::vector<province *> &path_branch)
{
	std::vector<const province *> path_branch_provinces;

	for (size_t i = 0; i < path_branch.size(); ++i) {
		province *path_province = path_branch[i];

		trade_route_path_element *path_element = nullptr;
		auto find_iterator = this->path.find(path_province);
		if (find_iterator != this->path.end()) {
			path_element = find_iterator->second.get();
		} else {
			auto new_path_element = std::make_unique<trade_route_path_element>(path_province);
			path_element = new_path_element.get();
			this->path[path_province] = std::move(new_path_element);
		}

		if (i > 0) {
			province *previous_path_province = path_branch[i - 1];
			path_element->add_to_previous(this->path[previous_path_province].get());
		}

		path_branch_provinces.push_back(path_province);
	}

	this->path_branch_provinces.push_back(std::move(path_branch_provinces));
}

void trade_route::clear_path()
{
	this->path.clear();
	this->path_endpoints.clear();
}

QVariantList trade_route::get_path_branch_points_qvariant_list() const
{
	QVariantList path_points_qvariant_list;

	for (const std::vector<const province *> &path_branch_provinces : this->path_branch_provinces) {
		QVariantList path_branch_points_qvariant_list;

		for (const province *path_province : path_branch_provinces) {
			path_branch_points_qvariant_list.append(path_province->get_center_pos() - this->get_rect().topLeft());
		}

		path_points_qvariant_list.push_back(path_branch_points_qvariant_list);
	}

	return path_points_qvariant_list;
}

void trade_route::set_active(const bool active)
{
	if (active == this->is_active()) {
		return;
	}

	this->active = active;
	emit active_changed();

	for (const auto &kv_pair : this->path) {
		province *path_province = kv_pair.first;
		if (active) {
			path_province->add_active_trade_route(this);
		} else {
			path_province->remove_active_trade_route(this);
		}
	}
}

void trade_route::calculate_active()
{
	//the endpoints for a trade route must have an owner (i.e. be settled) for the trade route to be active
	for (const province *endpoint_province : this->path_endpoints) {
		if (endpoint_province->get_owner() == nullptr) {
			this->set_active(false);
			return;
		}
	}

	this->set_active(true);
}

}
