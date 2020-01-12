#include "economy/trade_route.h"

#include "map/province.h"
#include "map/world.h"
#include "util/container_util.h"

namespace metternich {

void trade_route::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "path") {
		this->clear_path();
		for (const std::string &province_identifier : scope.get_values()) {
			province *path_province = province::get(province_identifier);
			this->add_path_province(path_province);
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
		if (path_province == nullptr) {
			throw std::runtime_error("The path of trade route \"" + this->get_identifier() + "\" contains a null province.");
		}

		path_province->add_trade_route(this);
	}

	province *start_province = this->path.front();

	if (this->get_world() == nullptr) {
		this->set_world(start_province->get_world());
	}

	if (!this->get_rect().isValid()) {
		QPoint top_left(-1, -1);
		QPoint bottom_right(-1, -1);

		for (province *path_province : this->path) {
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
	return container::to_qvariant_list(this->path);
}

void trade_route::add_path_province(province *path_province)
{
	this->path.push_back(path_province);
}

void trade_route::clear_path()
{
	this->path.clear();
}

QVariantList trade_route::get_path_points_qvariant_list() const
{
	QVariantList path_points;

	for (province *path_province : this->path) {
		path_points.append(path_province->get_center_pos() - this->get_rect().topLeft());
	}

	return path_points;
}

void trade_route::set_active(const bool active)
{
	if (active == this->is_active()) {
		return;
	}

	this->active = active;
	emit active_changed();

	for (province *path_province : this->path) {
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
	if (this->path.front()->get_owner() == nullptr || this->path.back()->get_owner() == nullptr) {
		this->set_active(false);
		return;
	}

	this->set_active(true);
}

}
