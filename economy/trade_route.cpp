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
	} else if (tag == "geopath") {
		this->set_geopath(scope.to_geopath());
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

	if (start_province->get_trade_node() == nullptr) {
		throw std::runtime_error("The start province of trade route \"" + this->get_identifier() + "\" has no trade node.");
	}

	this->add_trade_node(start_province->get_trade_node());

	province *end_province = this->path.back();

	if (end_province->get_trade_node() == nullptr) {
		throw std::runtime_error("The end province of trade route \"" + this->get_identifier() + "\" has no trade node.");
	}

	this->add_trade_node(end_province->get_trade_node());

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

void trade_route::set_geopath(const QGeoPath &geopath)
{
	if (geopath == this->get_geopath()) {
		return;
	}

	this->geopath = geopath;

	if (!this->geopath.isValid()) {
		throw std::runtime_error("Tried to set an invalid geopath for trade route \"" + this->get_identifier() + "\".");
	}

	//process the map as per the geopath data
	this->path.clear();

	for (const QGeoCoordinate &coordinate : this->geopath.path()) {
		province *path_province = this->get_world()->get_coordinate_province(coordinate);

		if (this->path.empty() && !path_province->is_center_of_trade()) {
			continue; //ignore provinces that aren't centers of trade for the first path element, as the wrong initial province could have been obtained simply due to the trade route start point being too close to the border
		}

		if (this->path.empty() || path_province != this->path.back()) {
			this->path.push_back(path_province);
		}
	}

	//ensure that the end province is a center of trade, as the wrong end province could have been set due to the trade route end point being too close to the border of the real end province
	while (!this->path.empty() && !this->path.back()->is_center_of_trade()) {
		this->path.pop_back();
	}

	const QGeoRectangle georectangle = this->geopath.boundingGeoRectangle();
	this->rect = this->get_world()->get_georectangle_rect(georectangle);
}

void trade_route::set_world(metternich::world *world)
{
	if (world == this->get_world()) {
		return;
	}

	this->world = world;
	world->add_trade_route(this);
}

void trade_route::add_trade_node(trade_node *node)
{
	if (node == nullptr) {
		throw std::runtime_error("Tried to add null as a trade node for trade route \"" + this->get_identifier() + "\".");
	}

	this->trade_nodes.insert(node);
	node->add_trade_route(this);
}

QVariantList trade_route::get_path_qvariant_list() const
{
	return container::to_qvariant_list(this->path);
}

QVariantList trade_route::get_path_points_qvariant_list() const
{
	QVariantList path_points;

	if (this->get_geopath().isValid()) {
		for (const QGeoCoordinate &coordinate : this->geopath.path()) {
			path_points.append(this->get_world()->get_coordinate_pos(coordinate));
		}
	} else {
		for (province *path_province : this->path) {
			path_points.append(path_province->get_center_pos());
		}
	}

	return path_points;
}

}
