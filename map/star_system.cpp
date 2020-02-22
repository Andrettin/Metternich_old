#include "map/star_system.h"

#include "engine_interface.h"
#include "landed_title/landed_title.h"
#include "map/map.h"
#include "map/map_edge.h"
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

	engine_interface::get()->set_loading_message("Calculating Star System Territories... (0%)");

	for (star_system *system : systems) {
		system->calculate_initial_territory_polygon();
	}

	const size_t total_system_count = systems.size();
	std::vector<star_system *> grown_systems;
	while (!systems.empty()) {
		for (star_system *system : systems) {
			const bool grew = system->grow_territory_polygon();
			if (grew) {
				grown_systems.push_back(system);
			}
		}

		if (grown_systems.size() != systems.size()) {
			engine_interface::get()->set_loading_message("Calculating Star System Territories... (" + QString::number((total_system_count - grown_systems.size()) * 100 / total_system_count) + "%)");

			systems = grown_systems;
		}

		grown_systems.clear();
	}
}

star_system::star_system(const std::string &identifier) : data_entry(identifier), map_edge(map_edge::none)
{
}

void star_system::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "adjacent_systems") {
		this->adjacent_systems.clear();
		for (const std::string &value : values) {
			this->adjacent_systems.push_back(star_system::get(value));
		}
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
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

	return translator::get()->translate(this->get_identifier_with_aliases()); //star system without a cosmic duchy
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

const QColor &star_system::get_color() const
{
	if (this->get_duchy() != nullptr) {
		return this->get_duchy()->get_color();
	}

	return star_system::empty_color;
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

void star_system::calculate_initial_territory_polygon()
{
	//create a polygon representing the star system's territory, using the middle points between this system's primary star and the primary stars of adjacent systems to build the polygon

	const double radius = (this->get_primary_star()->get_cosmic_size_with_satellites() / 2) + 32;
	const QPointF center_pos = this->get_primary_star()->get_cosmic_map_pos();
	this->territory_polygon = polygon::from_radius(radius, 1, center_pos);
	this->max_bounding_size = this->territory_polygon.boundingRect().width() + star_system::max_bounding_rect_offset;

	//calculate the nearby systems, for subsequent polygon growth calculation

	//the maximum distance to another system to consider it a nearby one
	const double max_nearby_distance = this->max_bounding_size;

	for (star_system *system : star_system::get_all()) {
		if (system == this) {
			continue;
		}

		const QPointF other_center_pos = system->get_primary_star()->get_cosmic_map_pos();
		const double distance = point::distance_to(center_pos, other_center_pos);

		if (distance > max_nearby_distance) {
			continue;
		}

		this->nearby_systems.insert(system);
		system->nearby_systems.insert(this);
	}
}

bool star_system::grow_territory_polygon()
{
	//return true if the polygon has grown, or false otherwise

	const int old_area = static_cast<int>(std::round(polygon::get_area(this->territory_polygon)));
	const QRectF old_bounding_rect = this->territory_polygon.boundingRect();

	if (old_bounding_rect.width() >= this->max_bounding_size || old_bounding_rect.height() >= this->max_bounding_size) {
		return false;
	}

	const QPointF center_pos = this->get_primary_star()->get_cosmic_map_pos();
	QPolygonF new_polygon;
	for (const QPointF &point : this->territory_polygon) {
		const double radius = point::distance_to(point, center_pos);
		const double new_radius = radius + star_system::territory_radius_growth;

		QPointF new_point(point::get_circle_point(point - center_pos, radius, new_radius) + center_pos);
		new_polygon.append(std::move(new_point));
	}

	for (const star_system *adj_system : this->nearby_systems) {
		const QRectF bounding_rect = new_polygon.boundingRect();
		const QRectF adj_bounding_rect = adj_system->territory_polygon.boundingRect();
		if (bounding_rect.intersects(adj_bounding_rect) && new_polygon.intersects(adj_system->territory_polygon)) {
			new_polygon = new_polygon.subtracted(adj_system->territory_polygon);
		}
	}

	const QRectF bounding_rect = new_polygon.boundingRect();
	const QRectF &map_bounding_rect = map::get()->get_cosmic_map_bounding_rect();
	if (!map_bounding_rect.contains(bounding_rect)) {
		new_polygon = new_polygon.intersected(map_bounding_rect);
	}

	const int new_area = static_cast<int>(std::round(polygon::get_area(new_polygon)));
	if (new_area > old_area) {
		this->territory_polygon = new_polygon;
		return true;
	}

	return false;
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
