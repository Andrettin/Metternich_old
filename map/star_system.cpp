#include "map/star_system.h"

#include "engine_interface.h"
#include "landed_title/landed_title.h"
#include "map/map.h"
#include "map/map_mode.h"
#include "map/world.h"
#include "translator.h"
#include "util/container_util.h"
#include "util/point_util.h"
#include "util/polygon_util.h"
#include "util/vector_util.h"

namespace metternich {

void star_system::calculate_territory_polygons()
{
	std::vector<star_system *> systems = star_system::get_all();

	if (systems.empty()) {
		return;
	}

	engine_interface::get()->set_loading_message("Calculating Star System Territories...");

	for (star_system *system : systems) {
		system->calculate_territory_polygon();
	}
}

star_system::star_system(const std::string &identifier) : data_entry(identifier)
{
}

void star_system::initialize()
{
	if (this->get_primary_star() == nullptr) {
		this->calculate_primary_star();
	}
}

std::string star_system::get_name() const
{
	if (this->get_duchy() != nullptr) {
		return translator::get()->translate(this->get_duchy()->get_identifier_with_aliases());
	}

	return translator::get()->translate(this->get_identifier_with_aliases()); //star system without a duchy
}

void star_system::set_duchy(landed_title *duchy)
{
	if (duchy == this->get_duchy()) {
		return;
	}

	this->duchy = duchy;
	duchy->set_star_system(this);
	emit duchy_changed();
}

landed_title *star_system::get_kingdom() const
{
	if (this->get_duchy() != nullptr) {
		return this->get_duchy()->get_kingdom();
	}

	return nullptr;
}

landed_title *star_system::get_de_jure_kingdom() const
{
	if (this->get_duchy() != nullptr) {
		return this->get_duchy()->get_de_jure_kingdom();
	}

	return nullptr;
}

landed_title *star_system::get_empire() const
{
	if (this->get_duchy() != nullptr) {
		return this->get_duchy()->get_empire();
	}

	return nullptr;
}

landed_title *star_system::get_de_jure_empire() const
{
	if (this->get_duchy() != nullptr) {
		return this->get_duchy()->get_de_jure_empire();
	}

	return nullptr;
}

void star_system::calculate_primary_star()
{
	this->primary_star = nullptr;

	for (world *world : this->get_worlds()) {
		if (world->get_orbit_center() == nullptr) {
			if (this->get_primary_star() != nullptr) {
				throw std::runtime_error("Star system \"" + this->get_identifier() + "\" has two celestial bodies orbiting no other one, making it impossible to determine its primary star.");
			}

			this->primary_star = world;
		}
	}

	if (this->get_primary_star() == nullptr) {
		throw std::runtime_error("Star system \"" + this->get_identifier() + "\" has no celestial bodies orbiting no other one, and thus no candidate for primary star.");
	}
}

const QColor &star_system::get_color_for_map_mode(const map_mode mode) const
{
	if (this->get_duchy() != nullptr) {
		switch (mode) {
			case map_mode::country: {
				const landed_title *realm = this->get_duchy()->get_realm();
				if (realm != nullptr) {
					return realm->get_color();
				}
				break;
			}
			case map_mode::de_jure_empire: {
				const landed_title *empire = this->get_de_jure_empire();
				if (empire != nullptr) {
					return empire->get_color();
				}
				break;
			}
			case map_mode::de_jure_kingdom: {
				const landed_title *kingdom = this->get_de_jure_empire();
				if (kingdom != nullptr) {
					return kingdom->get_color();
				}
				break;
			}
			case map_mode::de_jure_duchy: {
				const landed_title *duchy = this->get_duchy();
				if (duchy != nullptr) {
					return duchy->get_color();
				}
				break;
			}
			/*
			case map_mode::culture: {
				if (this->get_culture() != nullptr) {
					return this->get_culture()->get_color();
				}
				break;
			}
			case map_mode::culture_group: {
				if (this->get_culture() != nullptr) {
					return this->get_culture()->get_culture_group()->get_color();
				}
				break;
			}
			case map_mode::religion: {
				if (this->get_religion() != nullptr) {
					return this->get_religion()->get_color();
				}
				break;
			}
			case map_mode::religion_group: {
				if (this->get_religion() != nullptr) {
					return this->get_religion()->get_religion_group()->get_color();
				}
				break;
			}
			case map_mode::trade_node: {
				if (this->get_trade_node() != nullptr && this->get_owner() != nullptr) {
					return this->get_trade_node()->get_color();
				}
				break;
			}
			case map_mode::trade_zone: {
				if (this->get_trading_post_holding() != nullptr) {
					return this->get_trading_post_holding()->get_owner()->get_realm()->get_color();
				}
				break;
			}
			*/
			default:
				break;
		}
	}

	return star_system::empty_color; //unowned system
}

QPointF star_system::get_center_pos() const
{
	return this->get_primary_star()->get_cosmic_map_pos();
}

QVariantList star_system::get_territory_polygon_qvariant_list() const
{
	const QRectF bounding_rect = this->get_territory_bounding_rect();
	const QPolygonF polygon = this->territory_polygon.translated(QPointF(bounding_rect.x() * -1, bounding_rect.y() * -1));

	QVariantList polygon_qvariant_list;
	for (const QPointF &pos : polygon) {
		polygon_qvariant_list.append(QVariant::fromValue(QPointF(pos.x(), bounding_rect.height() - pos.y())));
	}

	return polygon_qvariant_list;
}

void star_system::calculate_territory_polygon()
{
	//create a polygon representing the star system's territory

	std::vector<star_system *> systems = star_system::get_all();

	//sort systems by distance to this one
	const QPointF center_pos = this->get_center_pos();
	std::sort(systems.begin(), systems.end(), [&center_pos](const star_system *a, const star_system *b) {
		const QPointF a_center_pos = a->get_center_pos();
		const QPointF b_center_pos = b->get_center_pos();
		const double a_distance = point::distance_to(a_center_pos, center_pos);
		const double b_distance = point::distance_to(b_center_pos, center_pos);
		return a_distance < b_distance;
	});

	const QRectF &map_rect = map::get()->get_cosmic_map_bounding_rect();
	const QLineF map_top(map_rect.topLeft(), map_rect.topRight());
	const QLineF map_bottom(map_rect.bottomLeft(), map_rect.bottomRight());
	const QLineF map_left(map_rect.topLeft(), map_rect.bottomLeft());
	const QLineF map_right(map_rect.topRight(), map_rect.bottomRight());

	const double territory_radius = star_system::territory_radius;
	this->territory_polygon = polygon::from_radius(territory_radius, 1, this->get_center_pos());

	for (const star_system *system : systems) {
		if (system == this) {
			continue;
		}

		const QPointF other_center_pos = system->get_center_pos();

		const QPointF middle_pos((center_pos.x() + other_center_pos.x()) / 2, (center_pos.y() + other_center_pos.y()) / 2);

		QLineF line(middle_pos, center_pos);
		line = line.normalVector();
		line.setLength(std::max(map_rect.width(), map_rect.height()) * 2);
		line = QLineF(line.p2(), line.p1());
		line.setLength(line.length() * 2);

		QPolygonF adj_polygon;

		QPointF intersection_point;
		bool top_intersection = false;
		bool bottom_intersection = false;
		bool left_intersection = false;
		bool right_intersection = false;

		if (line.intersects(map_top, &intersection_point) == QLineF::BoundedIntersection) {
			top_intersection = true;
			adj_polygon.append(intersection_point);
		}
		if (line.intersects(map_bottom, &intersection_point) == QLineF::BoundedIntersection) {
			bottom_intersection = true;
			adj_polygon.append(intersection_point);
		}
		if (line.intersects(map_left, &intersection_point) == QLineF::BoundedIntersection) {
			left_intersection = true;
			adj_polygon.append(intersection_point);
		}
		if (line.intersects(map_right, &intersection_point) == QLineF::BoundedIntersection) {
			right_intersection = true;
			adj_polygon.append(intersection_point);
		}

		if (top_intersection && bottom_intersection) {
			if (center_pos.x() < other_center_pos.x()) {
				adj_polygon.append(map_rect.bottomRight());
				adj_polygon.append(map_rect.topRight());
			} else {
				adj_polygon.append(map_rect.bottomLeft());
				adj_polygon.append(map_rect.topLeft());
			}
		} else if (left_intersection && right_intersection) {
			if (center_pos.y() < other_center_pos.y()) {
				adj_polygon.append(map_rect.bottomRight());
				adj_polygon.append(map_rect.bottomLeft());
			} else {
				adj_polygon.append(map_rect.topRight());
				adj_polygon.append(map_rect.topLeft());
			}
		} else if (top_intersection && left_intersection) {
			if (center_pos.x() < other_center_pos.x()) {
				adj_polygon.append(map_rect.bottomLeft());
				adj_polygon.append(map_rect.bottomRight());
				adj_polygon.append(map_rect.topRight());
			} else {
				adj_polygon.append(map_rect.topLeft());
			}
		} else if (top_intersection && right_intersection) {
			if (center_pos.x() < other_center_pos.x()) {
				adj_polygon.append(map_rect.topRight());
			} else {
				adj_polygon.append(map_rect.bottomRight());
				adj_polygon.append(map_rect.bottomLeft());
				adj_polygon.append(map_rect.topLeft());
			}
		} else if (bottom_intersection && left_intersection) {
			if (center_pos.x() < other_center_pos.x()) {
				adj_polygon.append(map_rect.topLeft());
				adj_polygon.append(map_rect.topRight());
				adj_polygon.append(map_rect.bottomRight());
			} else {
				adj_polygon.append(map_rect.bottomLeft());
			}
		} else if (bottom_intersection && right_intersection) {
			if (center_pos.x() < other_center_pos.x()) {
				adj_polygon.append(map_rect.bottomRight());
			} else {
				adj_polygon.append(map_rect.topRight());
				adj_polygon.append(map_rect.topLeft());
				adj_polygon.append(map_rect.bottomLeft());
			}
		} else {
			qWarning() << QString::fromStdString("The territory border line between star systems \"" + this->get_identifier() + "\" and \"" + system->get_identifier() + "\" has an invalid set of intersections with the map boundaries.");
		}

		adj_polygon.append(adj_polygon.front());  //close the polygon

		if (this->territory_polygon.intersects(adj_polygon)) {
			this->territory_polygon = this->territory_polygon.subtracted(adj_polygon);
		}

		//keep the territory polygon within the boundaries of the map
		if (!map_rect.contains(this->territory_polygon.boundingRect())) {
			this->territory_polygon = this->territory_polygon.intersected(map_rect);
		}
	}
}

QVariantList star_system::get_worlds_qvariant_list() const
{
	return container::to_qvariant_list(this->get_worlds());
}

void star_system::remove_world(world *world)
{
	vector::remove(this->worlds, world);
}

}
