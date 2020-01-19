#include "economy/trade_route.h"

#include "holding/holding_slot.h"
#include "map/province.h"
#include "map/world.h"
#include "util/container_util.h"
#include "util/map_util.h"
#include "util/point_util.h"
#include "util/set_util.h"

namespace metternich {


class trade_route::path_element
{
public:
	path_element(province *province) : province(province)
	{
	}

	province *get_province() const
	{
		return this->province;
	}

	bool has_connection_to(const province *province) const
	{
		for (const path_element *element : this->get_previous()) {
			if (element->get_province() == province) {
				return true;
			}
		}

		for (const path_element *element : this->get_next()) {
			if (element->get_province() == province) {
				return true;
			}
		}

		return false;
	}

	bool has_any_land_connection() const
	{
		for (const path_element *element : this->get_previous()) {
			if (element->get_province()->is_land()) {
				return true;
			}
		}

		for (const path_element *element : this->get_next()) {
			if (element->get_province()->is_land()) {
				return true;
			}
		}

		return false;
	}

	const std::set<path_element *> &get_previous() const
	{
		return this->previous;
	}

	void add_to_previous(path_element *element)
	{
		this->previous.insert(element);

		if (!element->next.contains(this)) {
			element->next.insert(this);
		}
	}

	const std::set<path_element *> &get_next() const
	{
		return this->next;
	}

private:
	province *province = nullptr; //the province for this path element
	std::set<path_element *> previous; //path elements immediately preceding this one
	std::set<path_element *> next; //path elements immediately succeeding this one
};

void trade_route::add_path_points_to_qvariantlist(QVariantList &point_list, const std::vector<const province *> &path_provinces, const QPoint &start_map_pos)
{
	QVariantList path_point_list;

	for (size_t i = 0; i < path_provinces.size(); ++i) {
		const province *path_province = path_provinces[i];
		if ((i + 1) < path_provinces.size()) {
			const province *next_path_province = path_provinces[i + 1];

			const std::vector<QPoint> &path_pos_list = path_province->get_path_pos_list(next_path_province);
			if (!path_pos_list.empty()) {
				for (const QPoint &path_pos : path_pos_list) {
					path_point_list.append(path_pos - start_map_pos);
				}

				if ((i + 2) >= path_provinces.size()) {
					//if the next province in the path is the last, and we had a pos path to it, then there's no need to continue, as that would draw that province's main pos (i.e. the pos path wouldn't be taken into account)
					break;
				}

				continue;
			}
		}

		const QPoint &main_pos = path_province->get_main_pos();
		path_point_list.append(main_pos - start_map_pos);
	}

	point_list.push_back(path_point_list);
}

trade_route::trade_route(const std::string &identifier) : data_entry(identifier)
{
}

trade_route::~trade_route()
{
}

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

	for (province *path_province : this->provinces) {
		if (path_province == nullptr) {
			throw std::runtime_error("The path of trade route \"" + this->get_identifier() + "\" contains a null province.");
		}

		path_province->add_trade_route(this);

		trade_route::path_element *path_element = this->get_province_path_element(path_province);
		if (path_element->get_previous().empty() || path_element->get_next().empty()) {
			this->path_endpoints.insert(path_province);
		}

		connect(path_province, &province::main_pos_changed, this, &trade_route::path_branch_points_changed);
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
			const QRect &province_rect = path_province->get_rect();

			if (top_left.x() == -1 || province_rect.x() < top_left.x()) {
				top_left.setX(province_rect.x());
			}

			if (top_left.y() == -1 || province_rect.y() < top_left.y()) {
				top_left.setY(province_rect.y());
			}

			if (bottom_right.x() == -1 || province_rect.right() > bottom_right.x()) {
				bottom_right.setX(province_rect.right());
			}

			if (bottom_right.y() == -1 || province_rect.bottom() > bottom_right.y()) {
				bottom_right.setY(province_rect.bottom());
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
	return container::to_qvariant_list(this->provinces);
}

void trade_route::add_path_branch(const std::vector<province *> &path_branch)
{
	std::vector<const province *> path_branch_provinces;

	for (size_t i = 0; i < path_branch.size(); ++i) {
		province *path_province = path_branch[i];

		this->provinces.insert(path_province);

		trade_route::path_element *path_element = nullptr;
		auto find_iterator = this->path.find(path_province);
		if (find_iterator != this->path.end()) {
			path_element = find_iterator->second.get();
		} else {
			auto new_path_element = std::make_unique<trade_route::path_element>(path_province);
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
	QVariantList path_points_list;

	for (const std::vector<const province *> &path_branch_provinces : this->path_branch_provinces) {
		trade_route::add_path_points_to_qvariantlist(path_points_list, path_branch_provinces, this->get_rect().topLeft());
	}

	return path_points_list;
}

QString trade_route::get_path_branch_points_svg() const
{
	QString svg;

	const QVariantList path_branches_list = this->get_path_branch_points_qvariant_list();

	for (const QVariant &path_branch_variant : path_branches_list) {
		const QVariantList path_branch = path_branch_variant.toList();

		for (int i = 0; i < path_branch.size(); ++i) {
			const QVariant &point_variant = path_branch[i];
			const QPoint point = point_variant.toPoint();

			if (i == 0) {
				svg += "M ";
			} else {
				svg += "L ";
			}

			svg += QString::number(point.x()) + " ";
			svg += QString::number(point.y()) + " ";
		}
	}

	return svg;
}

bool trade_route::has_connection_between(const province *source_province, const province *target_province) const
{
	return this->get_province_path_element(source_province)->has_connection_to(target_province);
}

bool trade_route::has_any_land_connection_for_province(const province *province) const
{
	return this->get_province_path_element(province)->has_any_land_connection();
}

void trade_route::set_active(const bool active)
{
	if (active == this->is_active()) {
		return;
	}

	this->active = active;
	emit active_changed();

	for (province *path_province : this->provinces) {
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
